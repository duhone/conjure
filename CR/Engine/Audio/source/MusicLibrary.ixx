module;

#include "core/Log.h"

export module CR.Engine.Audio.MusicLibrary;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Compression;

import CR.Engine.Audio.Constants;
import CR.Engine.Audio.Sample;
import CR.Engine.Audio.Utilities;

import <filesystem>;
import <typeindex>;
import <span>;
import <string>;
import <vector>;
import <unordered_map>;

namespace CR::Engine::Audio {
	export class MusicLibrary {
	  public:
		static std::type_index s_typeIndex;

		MusicLibrary();

		void Play(uint64_t a_nameHash) {
			uint16_t index = GetIndex(a_nameHash);
			CR_ASSERT(index < m_pcmData.size(), "Trying to play an invalid FX {}", index);
			m_playRequest([index](std::optional<uint16_t>& a_request) { a_request.emplace(index); });
		}

		void Mix(std::span<Sample> a_data);

		void SetVolume(float a_volume) { m_volume = CR::Engine::Audio::CalcVolume(a_volume); }

	  private:
		// quarter second
		constexpr inline static uint32_t c_transitionSamples = CR::Engine::Audio::c_mixSampleRate / 4;
		constexpr inline static float c_fadeStep             = 1.0f / c_transitionSamples;

		uint16_t GetIndex(uint64_t a_nameHash) const noexcept {
			auto iter = m_lookup.find(a_nameHash);
			CR_ASSERT(iter != m_lookup.end(), "Could not find audio music asset {}", a_nameHash);
			return iter->second;
		}

		std::unordered_map<uint64_t, uint16_t> m_lookup;
		std::vector<std::string> m_paths;
		std::vector<CR::Engine::Core::StorageBuffer<int16_t>> m_pcmData;

		CR::Engine::Core::Locked<std::optional<uint16_t>> m_playRequest;

		struct Playing {
			uint32_t Offset;
			uint16_t Index;
		};
		std::optional<Playing> m_playing;
		std::optional<uint16_t> m_pending;
		uint32_t m_transition{0};
		float m_volume{1.0f};
	};
}    // namespace CR::Engine::Audio

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace cea     = CR::Engine::Audio;
namespace cecomp  = CR::Engine::Compression;

namespace fs = std::filesystem;

std::type_index cea::MusicLibrary::s_typeIndex{typeid(MusicLibrary)};

cea::MusicLibrary::MusicLibrary() {
	auto& assetService = cecore::GetService<ceasset::Service>();
	assetService.Load(ceasset::Service::Partitions::Audio, "Music", "wav",
	                  [&](uint64_t a_hash, std::string_view a_path, const std::span<std::byte> a_data) {
		                  CR_ASSERT(m_pcmData.size() < std::numeric_limits<uint16_t>::max(),
		                            "Too many music files, max supported is 64K");
		                  m_lookup[a_hash] = (uint16_t)m_pcmData.size();
		                  m_paths.push_back(std::string(a_path));
		                  m_pcmData.push_back(cecomp::Wave::Decompress(a_data, a_path));
	                  });
}

void cea::MusicLibrary::Mix(std::span<Sample> a_data) {
	m_playRequest([this](std::optional<uint16_t>& a_request) {
		if(a_request.has_value()) {
			if(m_playing.has_value()) {
				m_pending    = a_request;
				m_transition = c_transitionSamples;
			} else {
				m_playing.emplace(0, a_request.value());
			}
			a_request.reset();
		}
	});

	if(m_pending.has_value() && m_transition <= 0) {
		m_playing.emplace(0, m_pending.value());
		m_pending.reset();
	}

	if(!m_playing.has_value() && !m_pending.has_value()) { return; }
	CR_ASSERT(!(!m_playing.has_value() && m_pending.has_value()),
	          "Should not be possible to have a pending, but not playing music track");

	auto& pcmData = m_pcmData[m_playing.value().Index];
	auto offset   = m_playing.value().Offset;
	float fade    = (float)m_transition / c_transitionSamples;
	CR_ASSERT(!m_pending.has_value() || (fade > 0.0f && fade <= 1.0f), "unexpeded fade value");

	for(int32_t i = 0; i < a_data.size(); ++i) {
		float sample = pcmData[offset++];
		if(offset >= pcmData.size()) { offset = 0; }
		if(m_pending.has_value()) {
			sample *= fade;
			fade = std::max(0.0f, fade - c_fadeStep);
		}
		sample *= m_volume / std::numeric_limits<int16_t>::max();
		a_data[i].Left += sample;
		a_data[i].Right += sample;
	}

	m_playing.value().Offset = offset;
}
