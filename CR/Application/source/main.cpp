#include <function2/function2.hpp>

import CR.Engine.Assets;
import CR.Engine.Audio;
import CR.Engine.Core;
import CR.Engine.Platform;

import <chrono>;
import <cstdlib>;
import <filesystem>;
import <thread>;

namespace ceassets = CR::Engine::Assets;
namespace cea      = CR::Engine::Audio;
namespace cec      = CR::Engine::Core;
namespace cep      = CR::Engine::Platform;

namespace fs = std::filesystem;

using namespace std::literals;

int main(int, char*) {
	cec::ServicesStart();

	fs::current_path(cep::GetCurrentProcessPath());

	cec::LogSystem logSystem;

	bool done = false;
	cep::Window window("Conjure", 800, 600, [&done]() { done = true; });

	fs::path assetsPath = fs::canonical(ASSETS_FOLDER);

	cec::AddService<ceassets::Service>(assetsPath);

	cea::EngineStart(false, assetsPath / "Audio/FX", assetsPath / "Audio/Music");

	{
		auto handleFxs = cea::GetHandleFXs();
		auto music     = cea::GetHandleMusic();

		handleFxs.SetVolume(1.0f);
		music.SetVolume(0.75f);

		auto fanfareFX = cea::GetHandleFX(cec::C_Hash64("FX/levelupfanfare.wav"));
		fanfareFX.Play();

		music.Play(cec::C_Hash64("Music/BGM_Menu.wav"));

		while(!done) { std::this_thread::sleep_for(16ms); }
	}

	cea::EngineStop();

	cec::ServicesStop();

	return EXIT_SUCCESS;
}