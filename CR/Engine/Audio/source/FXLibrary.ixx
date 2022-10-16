module;

#include "core/Log.h"

export module CR.Engine.Audio.FXLibrary;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Compression;

import <filesystem>;
import <numeric>;
import <typeindex>;
import <string>;
import <vector>;
import <unordered_map>;

namespace CR::Engine::Audio {
	export class FXLibrary {
	  public:
		static std::type_index s_typeIndex;

		FXLibrary(std::filesystem::path a_folder);

		uint16_t GetIndex(uint64_t a_nameHash) const noexcept {
			auto iter = m_lookup.find(a_nameHash);
			CR_ASSERT(iter != m_lookup.end(), "Could not find audio fx asset {}", a_nameHash);
			return iter->second;
		}

		void Play(uint16_t a_index) {
			m_playing([a_index](std::vector<Playing>& a_playing) { a_playing.emplace_back(0, a_index); });
		}

	  private:
		std::unordered_map<uint64_t, uint16_t> m_lookup;
		std::vector<std::string> m_paths;
		std::vector<CR::Engine::Core::StorageBuffer<int16_t>> m_pcmData;

		struct Playing {
			uint32_t Offset;
			uint16_t Index;
		};
		CR::Engine::Core::Locked<std::vector<Playing>> m_playing;
	};
}    // namespace CR::Engine::Audio

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace cea     = CR::Engine::Audio;
namespace cecomp  = CR::Engine::Compression;

namespace fs = std::filesystem;

std::type_index cea::FXLibrary::s_typeIndex{typeid(FXLibrary)};

cea::FXLibrary::FXLibrary(std::filesystem::path a_folder) {
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