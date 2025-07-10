module;

#include "core/Log.h"

#include <function2/function2.hpp>

export module CR.Engine.Audio.Service;

import CR.Engine.Core;

import CR.Engine.Audio.AudioDevice;
import CR.Engine.Audio.ChannelWeights;
import CR.Engine.Audio.Constants;
import CR.Engine.Audio.FXLibrary;
import CR.Engine.Audio.MusicLibrary;
import CR.Engine.Audio.OutputConversion;
import CR.Engine.Audio.Sample;

import <algorithm>;
import <filesystem>;
import <memory>;
import <span>;
import <ranges>;
import <typeindex>;
import <vector>;

namespace CR::Engine::Audio {
	export class Service {
	  public:
		static inline constexpr uint64_t s_typeIndex = CR::Engine::Core::EightCC("EAudServ");

		Service(bool a_checkForClipping);
		~Service()              = default;
		Service(const Service&) = delete;
		Service(Service&&)      = delete;

		Service& operator=(const Service&) = delete;
		Service& operator=(Service&&)      = delete;

		void Stop();

	  private:
		bool Mix(std::span<float>& a_buffer, int32_t a_numChannels, int32_t a_sampleRate,
		         const std::vector<ChannelWeights> a_weights, bool a_closing);

		std::unique_ptr<AudioDevice> m_device;
		std::vector<Sample> m_mixBuffer;
		// Holds audio after main buffer conversion to device sample rate, if needed
		std::vector<Sample> m_deviceSampleBuffer;
		std::vector<float> m_deviceChannelBuffer;

		OutputConversion m_outputConversion;

		bool m_checkForClipping;
	};
}    // namespace CR::Engine::Audio

module :private;

namespace cecore = CR::Engine::Core;
namespace ceaud  = CR::Engine::Audio;

bool ceaud::Service::Mix(std::span<float>& a_buffer, int32_t a_numChannels, int32_t a_sampleRate,
                         const std::vector<ChannelWeights> a_weights, bool a_closing) {
	int32_t mixBufferSize =
	    static_cast<int32_t>((a_buffer.size() * c_mixSampleRate) / (a_sampleRate * a_numChannels));
	m_mixBuffer.resize(mixBufferSize);
	std::ranges::fill(m_mixBuffer, Sample{});
	cecore::GetService<FXLibrary>().Mix({m_mixBuffer.data(), m_mixBuffer.size()});
	cecore::GetService<MusicLibrary>().Mix({m_mixBuffer.data(), m_mixBuffer.size()});

	std::span<Sample> resampleBuffer;
	if(a_sampleRate == c_mixSampleRate) {
		resampleBuffer = {m_mixBuffer.data(), m_mixBuffer.size()};
	} else {
		m_deviceSampleBuffer.resize(m_mixBuffer.size() * (a_sampleRate / c_mixSampleRate));
		resampleBuffer = {m_deviceSampleBuffer.data(), m_deviceSampleBuffer.size()};
		m_outputConversion.ConvertSampleRate(a_sampleRate, {m_mixBuffer.data(), m_mixBuffer.size()},
		                                     resampleBuffer);
	}

	std::span<float> deviceChannelBuffer;
	if(a_numChannels == c_mixChannels) {
		deviceChannelBuffer = {(float*)resampleBuffer.data(), resampleBuffer.size() * c_mixChannels};
	} else {
		m_deviceChannelBuffer.resize(resampleBuffer.size() * a_numChannels);
		deviceChannelBuffer = {m_deviceChannelBuffer.data(), m_deviceChannelBuffer.size()};
		m_outputConversion.ConvertChannelCount(resampleBuffer, deviceChannelBuffer, a_weights);
	}

	CR_ASSERT(deviceChannelBuffer.size() == a_buffer.size(),
	          "Logic error, final buffer did not match device requirements");

	if(m_checkForClipping) {
		for(const float& val : deviceChannelBuffer) {
			CR_ASSERT(val >= -1.0f && val < 1.0f, "audio clipping");
		}
	}

	memcpy(a_buffer.data(), deviceChannelBuffer.data(), a_buffer.size() * sizeof(float));

	if(a_closing) {
		return true;
	} else {
		return false;
	}
	return true;
}

ceaud::Service::Service(bool a_checkForClipping) {
	cecore::AddService<FXLibrary>();
	cecore::AddService<MusicLibrary>();

	m_checkForClipping = a_checkForClipping;

	m_device = std::make_unique<AudioDevice>(
	    [this](std::span<float> a_buffer, int32_t a_numChannels, int32_t a_sampleRate,
	           const std::vector<ChannelWeights> a_weights,
	           bool a_closing) { return Mix(a_buffer, a_numChannels, a_sampleRate, a_weights, a_closing); });
}

void ceaud::Service::Stop() {
	m_device.reset();
}
