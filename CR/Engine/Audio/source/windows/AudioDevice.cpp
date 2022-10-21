module;

#include "core/Log.h"
#include <function2/function2.hpp>
#include <platform/windows/CRWindows.h>

#include <atlbase.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <rtworkq.h>

module CR.Engine.Audio.AudioDevice;

import CR.Engine.Audio.Constants;

import CR.Engine.Core;

import <chrono>;
import <cstdint>;
import <thread>;

namespace cec = CR::Engine::Core;
namespace cea = CR::Engine::Audio;

using namespace std::chrono_literals;

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator    = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient3          = __uuidof(IAudioClient3);
const IID IID_IAudioRenderClient     = __uuidof(IAudioRenderClient);

namespace CR::Engine::Audio {
	class AudioDeviceImpl : public IRtwqAsyncCallback {
	  public:
		AudioDeviceImpl(AudioDevice::DeviceCallback_t a_callback);
		virtual ~AudioDeviceImpl();

		AudioDeviceImpl(const AudioDeviceImpl&)            = delete;
		AudioDeviceImpl(AudioDeviceImpl&&)                 = delete;
		AudioDeviceImpl& operator=(const AudioDeviceImpl&) = delete;
		AudioDeviceImpl& operator=(AudioDeviceImpl&&)      = delete;

		STDMETHODIMP GetParameters(DWORD* a_flags, DWORD* a_queue) override;
		STDMETHODIMP Invoke(IRtwqAsyncResult* a_result) override;

		STDMETHODIMP QueryInterface(const IID&, void**) override { return 0; }
		ULONG AddRef() override { return 0; }
		ULONG Release() override { return 0; }

		void PutWorkItem();

		void BuildChannelWeights(WAVEFORMATEXTENSIBLE* a_waveFormatDevice);
		ChannelWeights NormalizeChannelWeight(const ChannelWeights a_weight);

		CComPtr<IAudioClient3> m_audioClient;
		CComPtr<IAudioRenderClient> m_audioRenderClient;
		uint32_t m_frameSamples = 0;
		uint32_t m_bufferFrames = 0;
		uint32_t m_frameSize    = 0;
		uint32_t m_channels     = 0;
		uint32_t m_sampleRate   = 0;
		HANDLE m_audioEvent;

		DWORD m_rtWorkQueueId = 0;

		std::atomic<uint64_t> m_rtWorkItemKey = 0;
		std::atomic_bool m_finish             = false;
		std::atomic_bool m_finished           = false;
		std::atomic_int32_t m_finalFramesLeft = 4;

		AudioDevice::DeviceCallback_t m_callback;

		std::vector<ChannelWeights> m_channelWeights;
	};
}    // namespace CR::Engine::Audio

cea::AudioDeviceImpl::AudioDeviceImpl(AudioDevice::DeviceCallback_t a_callback) :
    m_callback(std::move(a_callback)) {
	HRESULT hr = CoInitialize(nullptr);
	CR_ASSERT(hr == S_OK, "Failed to initialize com for audio");

	CComPtr<IMMDeviceEnumerator> deviceEnumerator;
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator,
	                      (void**)&deviceEnumerator);

	CR_ASSERT(deviceEnumerator && hr == S_OK, "Failed to create audio device enumerator");

	CComPtr<IMMDevice> defaultDevice;
	hr = deviceEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, &defaultDevice);

	CR_ASSERT(defaultDevice && hr == S_OK, "Failed to create audio device");

	defaultDevice->Activate(IID_IAudioClient3, CLSCTX_ALL, nullptr, (void**)&m_audioClient);

	CR_ASSERT(m_audioClient && hr == S_OK, "Failed to create audio client");

	BOOL offloadCapable = false;
	m_audioClient->IsOffloadCapable(AUDIO_STREAM_CATEGORY::AudioCategory_GameEffects, &offloadCapable);

	AudioClientProperties audioProps;
	audioProps.cbSize     = sizeof(AudioClientProperties);
	audioProps.bIsOffload = offloadCapable;
	audioProps.eCategory  = AUDIO_STREAM_CATEGORY::AudioCategory_GameEffects;
	// Can probably save a little bit of latency by using a raw stream if supported. May not be desirable
	// though, would loose any user settings, equalizers, whatever.
	audioProps.Options = AUDCLNT_STREAMOPTIONS::AUDCLNT_STREAMOPTIONS_NONE;
	hr                 = m_audioClient->SetClientProperties(&audioProps);
	CR_ASSERT(hr == S_OK, "Failed to set wasapi audio client properties");

	WAVEFORMATEXTENSIBLE* waveFormatDevice = nullptr;
	m_audioClient->GetMixFormat((WAVEFORMATEX**)&waveFormatDevice);
	auto freeWaveFormat = cec::make_scope_exit([&]() { CoTaskMemFree(waveFormatDevice); });

	CR_LOG("\n**** Audio Device Properties Start ****");
	CR_LOG("OffloadCapable: {}", (offloadCapable == TRUE ? true : false));
	CR_LOG("Channels: {}", waveFormatDevice->Format.nChannels);
	CR_LOG("Samples Per Second: {}", waveFormatDevice->Format.nSamplesPerSec);
	CR_LOG("Bits Per Sample: {}", waveFormatDevice->Format.wBitsPerSample);
	if(waveFormatDevice->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
		CR_LOG("Valid Bits per Samples: {}", waveFormatDevice->Samples.wValidBitsPerSample);
		if(waveFormatDevice->SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
			CR_LOG("Format: PCM");
		} else if(waveFormatDevice->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
			CR_LOG("Format: Float");
		} else {
			CR_ERROR("Format: Unknown or Unsupported");
		}
	}

	if(waveFormatDevice->Format.wBitsPerSample != 32) {
		CR_ERROR("Unsupported number of bits per sample, must be float");
	}
	if(waveFormatDevice->Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
		CR_ERROR("require support for float audio format");
	}
	if(waveFormatDevice->SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
		CR_ERROR("require support for float audio format");
	}

	BuildChannelWeights(waveFormatDevice);

	UINT32 defaultPeriod     = 0;
	UINT32 fundamentalPeriod = 0;
	UINT32 minPeriod         = 0;
	UINT32 maxPeriod         = 0;

	hr = m_audioClient->GetSharedModeEnginePeriod(&waveFormatDevice->Format, &defaultPeriod,
	                                              &fundamentalPeriod, &minPeriod, &maxPeriod);
	CR_ASSERT(hr == S_OK, "Failed to retrieve the low latency audio period");
	CR_LOG("Default Period: {} Fundamental Period: {} Minimum Period: {} Maximum Period: {} Min "
	       "Period Time {}ms",
	       defaultPeriod, fundamentalPeriod, minPeriod, maxPeriod,
	       (1000 * minPeriod / waveFormatDevice->Format.nSamplesPerSec));

	CR_LOG("**** Audio Device Properties End ****\n");

	m_frameSize    = waveFormatDevice->Format.nBlockAlign;
	m_frameSamples = minPeriod;
	m_channels     = waveFormatDevice->Format.nChannels;
	m_sampleRate   = waveFormatDevice->Format.nSamplesPerSec;
	hr = m_audioClient->InitializeSharedAudioStream(AUDCLNT_STREAMFLAGS_EVENTCALLBACK, m_frameSamples,
	                                                &waveFormatDevice->Format, nullptr);
	CR_ASSERT(hr == S_OK, "Failed to set initialize wasapi audio stream");

	m_audioEvent = CreateEventA(nullptr, FALSE, FALSE, nullptr);
	CR_ASSERT(m_audioEvent != NULL, "Failed to create an event to use with event driven wasapi");
	m_audioClient->SetEventHandle(m_audioEvent);

	m_audioClient->GetBufferSize(&m_bufferFrames);

	hr = m_audioClient->GetService(IID_IAudioRenderClient, (void**)&m_audioRenderClient);
	CR_ASSERT(hr == S_OK && m_audioRenderClient, "Failed to create a wasapi render client");

	// prime with silence
	BYTE* buffer = nullptr;
	hr           = m_audioRenderClient->GetBuffer(m_bufferFrames, &buffer);
	CR_ASSERT(hr == S_OK, "Failed to get wasapi buffer");
	memset(buffer, 0, m_bufferFrames * m_frameSize);
	hr = m_audioRenderClient->ReleaseBuffer(m_bufferFrames, 0);
	CR_ASSERT(hr == S_OK, "Failed to release wasapi buffer");

	hr = RtwqStartup();
	CR_ASSERT(hr == S_OK, "Failed to startup real time work queues");

	m_rtWorkQueueId = RTWQ_MULTITHREADED_WORKQUEUE;

	DWORD taskId = 0;
	hr           = RtwqLockSharedWorkQueue(L"Pro Audio", 0, &taskId, &m_rtWorkQueueId);
	CR_ASSERT(hr == S_OK, "Failed to lock pro audio work queue");

	PutWorkItem();

	m_audioClient->Start();
}

cea::AudioDeviceImpl::~AudioDeviceImpl() {
	HRESULT hr;

	m_finish.store(true, std::memory_order_release);
	while(!m_finished.load(std::memory_order_acquire)) { std::this_thread::sleep_for(1ms); }

	m_audioClient->Stop();

	hr = RtwqUnlockWorkQueue(m_rtWorkQueueId);
	CR_ASSERT(hr == S_OK, "Failed to unlock pro audio work queue");
	hr = RtwqShutdown();
	CR_ASSERT(hr == S_OK, "Failed to shutdown real time work queues");

	CloseHandle(m_audioEvent);
	m_audioRenderClient.Release();
	m_audioClient.Release();
	CoUninitialize();
}

STDMETHODIMP cea::AudioDeviceImpl::GetParameters(DWORD* a_flags, DWORD* a_queue) {
	*a_flags = 0;
	*a_queue = m_rtWorkQueueId;

	return S_OK;
}

STDMETHODIMP cea::AudioDeviceImpl::Invoke(IRtwqAsyncResult*) {
	CR_ASSERT(m_finished.load(std::memory_order_acquire) != true,
	          "Should never have a work item queued, if the device is already finsished");

	HRESULT hr;

	uint32_t padding;
	hr = m_audioClient->GetCurrentPadding(&padding);
	CR_ASSERT(hr == S_OK, "Failed to get current padding");

	uint32_t numFrames = std::min(m_bufferFrames - padding, m_frameSamples);

	float* buffer = nullptr;
	hr            = m_audioRenderClient->GetBuffer(numFrames, (BYTE**)&buffer);
	CR_ASSERT(hr == S_OK, "Failed to get wasapi buffer");

	bool finish = m_finish.load(std::memory_order_acquire);

	bool fillSilence = m_callback(std::span<float>{buffer, numFrames * m_channels}, m_channels, m_sampleRate,
	                              m_channelWeights, finish);
	if(fillSilence) {
		hr = m_audioRenderClient->ReleaseBuffer(numFrames, AUDCLNT_BUFFERFLAGS_SILENT);
		CR_ASSERT(hr == S_OK, "Failed to release wasapi buffer");

	} else {
		hr = m_audioRenderClient->ReleaseBuffer(numFrames, 0);
		CR_ASSERT(hr == S_OK, "Failed to release wasapi buffer");
	}

	bool finished = m_finished.load(std::memory_order_acquire);
	if(finish && fillSilence && !finished) {
		uint32_t framesLeft = framesLeft = m_finalFramesLeft.fetch_add(-1);
		if(finish && framesLeft == 0) {
			m_finished.store(true, std::memory_order_release);
			finished = true;
		}
	}

	if(!finished) { PutWorkItem(); }

	return S_OK;
}

void cea::AudioDeviceImpl::PutWorkItem() {
	HRESULT hr;
	CComPtr<IRtwqAsyncResult> asyncResult;
	hr = RtwqCreateAsyncResult(nullptr, this, nullptr, &asyncResult);
	CR_ASSERT(hr == S_OK, "Failed to create real time work queue async result");

	uint64_t workItemKey;
	hr = RtwqPutWaitingWorkItem(m_audioEvent, 1, asyncResult, &workItemKey);
	m_rtWorkItemKey.store(workItemKey, std::memory_order_release);
	CR_ASSERT(hr == S_OK, "Failed to put a work item to real time work queue for audio");
}

cea::ChannelWeights cea::AudioDeviceImpl::NormalizeChannelWeight(const ChannelWeights a_weight) {
	ChannelWeights result = a_weight;
	float invMagnitude    = 1.0f / sqrt(result.Left * result.Left + result.Right * result.Right);
	result.Left *= invMagnitude;
	result.Right *= invMagnitude;
	return result;
}

void cea::AudioDeviceImpl::BuildChannelWeights(WAVEFORMATEXTENSIBLE* a_waveFormatDevice) {
	CR_ASSERT(a_waveFormatDevice->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE,
	          "Assume extensible wave format");
	DWORD channelMask = a_waveFormatDevice->dwChannelMask;
	// These must be checked/added in order they occur in windows header
	// see https://docs.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible
	// for required order
	if(channelMask & SPEAKER_FRONT_LEFT) { m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 0.0f})); }
	if(channelMask & SPEAKER_FRONT_RIGHT) {
		m_channelWeights.push_back(NormalizeChannelWeight({0.0f, 1.0f}));
	}
	if(channelMask & SPEAKER_FRONT_CENTER) {
		m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 1.0f}));
	}
	// This one should really have a low pass filter applied as well
	if(channelMask & SPEAKER_LOW_FREQUENCY) {
		m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 1.0f}));
	}
	if(channelMask & SPEAKER_BACK_LEFT) { m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 0.0f})); }
	if(channelMask & SPEAKER_BACK_RIGHT) { m_channelWeights.push_back(NormalizeChannelWeight({0.0f, 1.0f})); }
	if(channelMask & SPEAKER_FRONT_LEFT_OF_CENTER) {
		m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 0.0f}));
	}
	if(channelMask & SPEAKER_FRONT_RIGHT_OF_CENTER) {
		m_channelWeights.push_back(NormalizeChannelWeight({0.0f, 1.0f}));
	}
	if(channelMask & SPEAKER_BACK_CENTER) {
		m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 1.0f}));
	}
	if(channelMask & SPEAKER_SIDE_LEFT) { m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 0.0f})); }
	if(channelMask & SPEAKER_SIDE_RIGHT) { m_channelWeights.push_back(NormalizeChannelWeight({0.0f, 1.0f})); }
	if(channelMask & SPEAKER_TOP_CENTER) { m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 1.0f})); }
	if(channelMask & SPEAKER_TOP_FRONT_LEFT) {
		m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 0.0f}));
	}
	if(channelMask & SPEAKER_TOP_FRONT_CENTER) {
		m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 1.0f}));
	}
	if(channelMask & SPEAKER_TOP_FRONT_RIGHT) {
		m_channelWeights.push_back(NormalizeChannelWeight({0.0f, 1.0f}));
	}
	if(channelMask & SPEAKER_TOP_BACK_LEFT) {
		m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 0.0f}));
	}
	if(channelMask & SPEAKER_TOP_BACK_CENTER) {
		m_channelWeights.push_back(NormalizeChannelWeight({1.0f, 1.0f}));
	}
	if(channelMask & SPEAKER_TOP_BACK_RIGHT) {
		m_channelWeights.push_back(NormalizeChannelWeight({0.0f, 1.0f}));
	}
}

cea::AudioDevice::AudioDevice(AudioDevice::DeviceCallback_t a_callback) {
	m_pimpl = std::make_unique<AudioDeviceImpl>(std::move(a_callback));
}

cea::AudioDevice::~AudioDevice() {}
