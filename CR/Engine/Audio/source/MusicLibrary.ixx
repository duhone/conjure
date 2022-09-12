export module CR.Engine.Audio.MusicLibrary;

import CR.Engine.Core;
import CR.Engine.Compression;

import <filesystem>;
import <typeindex>;
import <string>;
import <vector>;
import <unordered_map>;

namespace CR::Engine::Audio {
	export class MusicLibrary {
	  public:
		static std::type_index s_typeIndex;

		MusicLibrary(std::filesystem::path a_folder);

	  private:
		std::unordered_map<std::string, uint32_t> m_lookup;
		std::vector<CR::Engine::Core::StorageBuffer<int16_t>> m_pcmData;
	};
}    // namespace CR::Engine::Audio

module :private;

namespace cea    = CR::Engine::Audio;
namespace cecomp = CR::Engine::Compression;

namespace fs = std::filesystem;

std::type_index cea::MusicLibrary::s_typeIndex{typeid(MusicLibrary)};

cea::MusicLibrary::MusicLibrary(std::filesystem::path a_folder) {
	for(const auto& dirEntry : fs::recursive_directory_iterator(a_folder)) {
		if(dirEntry.is_regular_file() && dirEntry.path().extension() == ".wav") {
			auto file = fs::relative(dirEntry.path(), a_folder);
			file.make_preferred();
			file.replace_extension();
			m_lookup[file.string()] = (uint32_t)m_pcmData.size();

			m_pcmData.push_back(cecomp::Wave::Decompress(dirEntry.path()));
		}
	}
}