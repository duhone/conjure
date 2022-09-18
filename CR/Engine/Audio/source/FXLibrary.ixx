module;

#include "core/Log.h"

export module CR.Engine.Audio.FXLibrary;

import CR.Engine.Core;
import CR.Engine.Compression;

import <filesystem>;
import <typeindex>;
import <string>;
import <vector>;
import <unordered_map>;

namespace CR::Engine::Audio {
	export class FXLibrary {
	  public:
		static std::type_index s_typeIndex;

		FXLibrary(std::filesystem::path a_folder);

		uint16_t GetIndex(const std::string_view a_name) const noexcept {
			auto iter = m_lookup.find(std::string(a_name));
			CR_ASSERT(iter != m_lookup.end(), "Could not find audio fx asset {}", a_name);
			return iter->second;
		}

		void Play(uint16_t a_index) { m_playing.emplace_back(0, a_index); }

	  private:
		std::unordered_map<std::string, uint16_t> m_lookup;
		std::vector<CR::Engine::Core::StorageBuffer<int16_t>> m_pcmData;

		struct Playing {
			uint32_t Offset;
			uint16_t Index;
		};
		std::vector<Playing> m_playing;
	};
}    // namespace CR::Engine::Audio

module :private;

namespace cea    = CR::Engine::Audio;
namespace cecomp = CR::Engine::Compression;

namespace fs = std::filesystem;

std::type_index cea::FXLibrary::s_typeIndex{typeid(FXLibrary)};

cea::FXLibrary::FXLibrary(std::filesystem::path a_folder) {
	for(const auto& dirEntry : fs::recursive_directory_iterator(a_folder)) {
		if(dirEntry.is_regular_file() && dirEntry.path().extension() == ".wav") {
			auto file = fs::relative(dirEntry.path(), a_folder);
			file.make_preferred();
			file.replace_extension();
			m_lookup[file.string()] = (uint16_t)m_pcmData.size();

			m_pcmData.push_back(cecomp::Wave::Decompress(dirEntry.path()));
		}
	}
}