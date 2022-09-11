export module CR.Engine.Audio.FXLibrary;

import<filesystem>;
import<typeindex>;

namespace CR::Engine::Audio {
	export class FXLibrary {
	  public:
		static std::type_index s_typeIndex;

		FXLibrary(std::filesystem::path a_folder);
	};
}    // namespace CR::Engine::Audio

module : private;

namespace cea = CR::Engine::Audio;

std::type_index cea::FXLibrary::s_typeIndex{typeid(FXLibrary)};

cea::FXLibrary::FXLibrary(std::filesystem::path) {}