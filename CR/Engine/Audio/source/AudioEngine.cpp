module;

#include "core/Log.h"

#include <function2/function2.hpp>

module CR.Engine.Audio.Engine;

import CR.Engine.Core;

import CR.Engine.Audio.AudioDevice;
import CR.Engine.Audio.ChannelWeights;
import CR.Engine.Audio.Constants;
import CR.Engine.Audio.FXLibrary;
import CR.Engine.Audio.MixerSystem;
import CR.Engine.Audio.MusicLibrary;
import CR.Engine.Audio.OutputConversion;
import CR.Engine.Audio.Sample;

import <algorithm>;
import <span>;
import <memory>;
import <ranges>;
import <vector>;

namespace cecore = CR::Engine::Core;
namespace cea    = CR::Engine::Audio;

namespace {
	struct Engine {
		bool Mix(std::span<float>& a_buffer, int32_t a_numChannels, int32_t a_sampleRate,
		         const std::vector<cea::ChannelWeights> a_weights, bool a_closing);

		std::unique_ptr<cea::AudioDevice> m_device;
		std::vector<cea::Sample> m_mixBuffer;
		// Holds audio after main buffer conversion to device sample rate, if needed
		std::vector<cea::Sample> m_deviceSampleBuffer;
		std::vector<float> m_deviceChannelBuffer;

		cea::OutputConversion m_outputConversion;

		cea::MixerSystem m_mixerSystem;

		bool m_checkForClipping;
	};
	Engine& GetEngine() {
		static Engine s_engine;
		return s_engine;
	}
}    // namespace

bool Engine::Mix(std::span<float>& a_buffer, int32_t a_numChannels, int32_t a_sampleRate,
                 const std::vector<cea::ChannelWeights> a_weights, bool a_closing) {
	int32_t mixBufferSize =
	    static_cast<int32_t>((a_buffer.size() * cea::c_mixSampleRate) / (a_sampleRate * a_numChannels));
	m_mixBuffer.resize(mixBufferSize);
	std::ranges::fill(m_mixBuffer, cea::Sample{});
	cecore::GetService<cea::FXLibrary>().Mix({m_mixBuffer.data(), m_mixBuffer.size()});

	std::span<cea::Sample> resampleBuffer;
	if(a_sampleRate == cea::c_mixSampleRate) {
		resampleBuffer = {m_mixBuffer.data(), m_mixBuffer.size()};
	} else {
		m_deviceSampleBuffer.resize(m_mixBuffer.size() * (a_sampleRate / cea::c_mixSampleRate));
		resampleBuffer = {m_deviceSampleBuffer.data(), m_deviceSampleBuffer.size()};
		m_outputConversion.ConvertSampleRate(a_sampleRate, {m_mixBuffer.data(), m_mixBuffer.size()},
		                                     resampleBuffer);
	}

	std::span<float> deviceChannelBuffer;
	if(a_numChannels == cea::c_mixChannels) {
		deviceChannelBuffer = {(float*)resampleBuffer.data(), resampleBuffer.size() * cea::c_mixChannels};
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
}

void cea::EngineStart(bool a_checkForClipping, std::filesystem::path a_fxFolder,
                      std::filesystem::path a_musicFolder) {
	cecore::AddService<FXLibrary>(std::move(a_fxFolder));
	cecore::AddService<MusicLibrary>(std::move(a_musicFolder));

	::Engine& engine = GetEngine();

	engine.m_checkForClipping = a_checkForClipping;
	// engine.m_masterMix        = engine.m_mixerSystem.CreateMixer();

	engine.m_device = std::make_unique<AudioDevice>(
	    [&engine](std::span<float> a_buffer, int32_t a_numChannels, int32_t a_sampleRate,
	              const std::vector<ChannelWeights> a_weights, bool a_closing) {
		    return engine.Mix(a_buffer, a_numChannels, a_sampleRate, a_weights, a_closing);
	    });
}

void cea::EngineStop() {
	::Engine& engine = GetEngine();

	engine.m_device.reset();
}
