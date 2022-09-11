#include <function2/function2.hpp>

import CR.Engine.Audio;
import CR.Engine.Core;
import CR.Engine.Platform;

import<chrono>;
import<cstdlib>;
import<filesystem>;
import<thread>;

namespace cea = CR::Engine::Audio;
namespace cec = CR::Engine::Core;
namespace cep = CR::Engine::Platform;

namespace fs = std::filesystem;

using namespace std::literals;

int main(int, char*) {
	fs::current_path(cep::GetCurrentProcessPath());

	cec::LogSystem logSystem;

	bool done = false;
	cep::Window window("Conjure", 800, 600, [&done]() { done = true; });

	fs::path assetsPath = fs::canonical(ASSETS_FOLDER);

	cea::EngineStart(false, assetsPath / "Audio/FX", assetsPath / "Audio/Music");

	{
		cea::Tone tone("440hz", 440.0f);
		tone.SetVolume(0.5f);
		cea::Tone tone2("880hz", 880.0f);
		tone2.SetVolume(0.35f);
		cea::Tone tone3("1760hz", 1760.0f);
		tone3.SetVolume(0.2f);

		while(!done) { std::this_thread::sleep_for(16ms); }
	}

	cea::EngineStop();

	return EXIT_SUCCESS;
}