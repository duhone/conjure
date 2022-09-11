export module CR.Engine.Audio.MusicLibrary;

import<filesystem>;
import<typeindex>;

namespace CR::Engine::Audio {
	export class MusicLibrary {
	  public:
		static std::type_index s_typeIndex;

		MusicLibrary(std::filesystem::path a_folder);
	};
}    // namespace CR::Engine::Audio

module : private;

namespace cea = CR::Engine::Audio;

std::type_index cea::MusicLibrary::s_typeIndex{typeid(MusicLibrary)};

cea::MusicLibrary::MusicLibrary(std::filesystem::path) {}