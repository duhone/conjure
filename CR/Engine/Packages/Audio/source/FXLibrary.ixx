module;

#include "generated/audio/soundfx_generated.h"

#include "flatbuffers/idl.h"

#include "core/Log.h"

#include <dr_flac.h>

export module CR.Engine.Audio.FXLibrary;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

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
		std::vector<std::string> m_names;
		std::vector<std::string> m_paths;
		std::vector<CR::Engine::Core::StorageBuffer<int16_t>> m_pcmData;

		struct Playing {
			uint32_t Offset;
			uint16_t Index;
		};
		CR::Engine::Core::Locked<std::vector<uint16_t>> m_playRequests;
		std::vector<Playing> m_playing;
		float m_volume{1.0f};
		bool m_mute{};
	};
}    // namespace CR::Engine::Audio

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace ceplat  = CR::Engine::Platform;
namespace cea     = CR::Engine::Audio;

namespace fs = std::filesystem;

cea::FXLibrary::FXLibrary() {
	auto& assetService = cecore::GetService<ceasset::Service>();

	flatbuffers::Parser parser =
	    assetService.GetData(cecore::C_Hash64("Audio/soundfx.json"), SCHEMAS_SOUNDFX);
	auto sounds = Flatbuffers::GetSoundFXs(parser.builder_.GetBufferPointer());
	for(const auto& soundfx : *sounds->soundfx()) {
		m_lookup[cecore::Hash64(soundfx->name()->c_str())] = (uint16_t)m_pcmData.size();
		m_names.push_back(soundfx->name()->c_str());
		m_paths.push_back(soundfx->path()->c_str());

		auto soundHandle = assetService.GetHandle(cecore::Hash64(m_paths.back()));
		assetService.Open(soundHandle);
		auto soundData = assetService.GetData(soundHandle);

		uint32_t uncompressedSize{};
		auto metaData = [](void* pUserData, drflac_metadata* pMetadata) {
			if(pMetadata->type == DRFLAC_METADATA_BLOCK_TYPE_STREAMINFO) {
				uint32_t* size = (uint32_t*)pUserData;
				*size          = (uint32_t)pMetadata->data.streaminfo.totalPCMFrameCount;
			}
		};
		auto drFlac = drflac_open_memory_with_metadata(soundData.data(), soundData.size(), metaData,
		                                               &uncompressedSize, nullptr);

		CR_ASSERT(uncompressedSize > 0, "0 size flac file");

		CR::Engine::Core::StorageBuffer<int16_t> pcmData;
		pcmData.prepare(uncompressedSize);

		auto framesRead = drflac_read_pcm_frames_s16(drFlac, uncompressedSize, pcmData.data());
		while(framesRead < uncompressedSize) {
			framesRead += drflac_read_pcm_frames_s16(drFlac, uncompressedSize - framesRead,
			                                         pcmData.data() + framesRead);
		}
		drflac_close(drFlac);

		pcmData.commit(uncompressedSize);

		m_pcmData.emplace_back(std::move(pcmData));

		assetService.Close(soundHandle);
	}
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
