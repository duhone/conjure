module;

#include "core/Log.h"

export module CR.Engine.Audio.FXLibrary;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Compression;

import CR.Engine.Audio.Sample;
import CR.Engine.Audio.Utilities;

import <algorithm>;
import <filesystem>;
import <limits>;
import <numeric>;
import <typeindex>;
import <span>;
import <string>;
import <vector>;
import <unordered_map>;

namespace CR::Engine::Audio {
	export class FXLibrary {
	  public:
		static inline constexpr uint64_t s_typeIndex = CR::Engine::Core::EightCC("EAudFxfx");

		FXLibrary();

		uint16_t GetIndex(uint64_t a_nameHash) const noexcept {
			auto iter = m_lookup.find(a_nameHash);
			CR_ASSERT(iter != m_lookup.end(), "Could not find audio fx asset {}", a_nameHash);
			return iter->second;
		}

		void Play(uint16_t a_index) {
			CR_ASSERT(a_index < m_pcmData.size(), "Trying to play an invalid FX {}", a_index);
			m_playRequests(
			    [a_index](std::vector<uint16_t>& a_requests) { a_requests.emplace_back(a_index); });
		}

		void Mix(std::span<Sample> a_data);

		float GetVolume() const { return CR::Engine::Audio::CalcLinearVolume(m_volume); }
		void SetVolume(float a_volume) { m_volume = CR::Engine::Audio::CalcLogVolume(a_volume); }
		bool GetMute() const { return m_mute; }
		void SetMute(bool a_mute) { m_mute = a_mute; }

	  private:
		std::unordered_map<uint64_t, uint16_t> m_lookup;
		std::vector<std::string> m_paths;
		std::vector<CR::Engine::Core::StorageBuffer<int16_t>> m_pcmData;

		struct Playing {
			uint32_t Offset;
			uint16_t Index;
		};
		CR::Engine::Core::Locked<std::vector<uint16_t>> m_playRequests;
		std::vector<Playing> m_playing;
		float m_volume{1.0f};
		bool m_mute;
	};
}    // namespace CR::Engine::Audio

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace cea     = CR::Engine::Audio;
namespace cecomp  = CR::Engine::Compression;

namespace fs = std::filesystem;

cea::FXLibrary::FXLibrary() {
	auto& assetService = cecore::GetService<ceasset::Service>();
	assetService.Load(ceasset::Service::Partitions::Audio, "FX", "wav",
	                  [&](uint64_t a_hash, std::string_view a_path, const std::span<std::byte> a_data) {
		                  CR_ASSERT(m_pcmData.size() < std::numeric_limits<uint16_t>::max(),
		                            "Too many fx, max supported is 64K");
		                  m_lookup[a_hash] = (uint16_t)m_pcmData.size();
		                  m_paths.push_back(std::string(a_path));
		                  m_pcmData.push_back(cecomp::Wave::Decompress(a_data, a_path));
	                  });
}

void cea::FXLibrary::Mix(std::span<Sample> a_data) {
	m_playRequests([this](std::vector<uint16_t>& a_requests) {
		for(auto index : a_requests) { m_playing.emplace_back(0, index); }
		a_requests.clear();
	});

	for(auto& playing : m_playing) {
		CR_ASSERT_AUDIT(playing.Index < m_pcmData.size(),
		                "invalid playing audio, should have been caught in Play call");
		auto& pcmData = m_pcmData[playing.Index];
		auto toMix    = std::min<int32_t>((int32_t)pcmData.size() - playing.Offset, (int32_t)a_data.size());
		for(int32_t i = 0; i < toMix; ++i) {
			float sample = pcmData[playing.Offset + i];
			sample *= m_volume / std::numeric_limits<int16_t>::max();
			if(m_mute) { sample = 0.0f; }
			a_data[i].Left += sample;
			a_data[i].Right += sample;
		}
		playing.Offset += toMix;
	}

	std::erase_if(m_playing,
	              [this](auto& playing) { return playing.Offset >= m_pcmData[playing.Index].size(); });
}
