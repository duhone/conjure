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
namespace ceaud    = CR::Engine::Audio;
namespace ceccore  = CR::Engine::Core;
namespace ceplat   = CR::Engine::Platform;

namespace fs = std::filesystem;

using namespace std::literals;

int main(int, char*) {
	ceccore::ServicesStart();

	fs::current_path(ceplat::GetCurrentProcessPath());

	ceccore::LogSystem logSystem;

	bool done = false;
	ceplat::Window window("Conjure", 800, 600, [&done]() { done = true; });

	fs::path assetsPath = fs::canonical(ASSETS_FOLDER);

	ceccore::AddService<ceassets::Service>(assetsPath);
	ceccore::AddService<ceaud::Service>(false);

	{
		auto handleFxs = ceaud::GetHandleFXs();
		auto music     = ceaud::GetHandleMusic();

		handleFxs.SetVolume(1.0f);
		music.SetVolume(0.75f);

		auto fanfareFX = ceaud::GetHandleFX(ceccore::C_Hash64("FX/levelupfanfare.wav"));
		fanfareFX.Play();

		music.Play(ceccore::C_Hash64("Music/BGM_Menu.wav"));

		while(!done) { std::this_thread::sleep_for(16ms); }
	}

	ceccore::ServicesStop();

	return EXIT_SUCCESS;
}