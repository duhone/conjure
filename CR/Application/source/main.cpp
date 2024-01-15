#include <function2/function2.hpp>

#include <glm/glm.hpp>

import CR.Engine.Assets;
import CR.Engine.Audio;
import CR.Engine.Core;
import CR.Engine.Input;
import CR.Engine.Graphics;
import CR.Engine.Platform;

import <chrono>;
import <cstdlib>;
import <filesystem>;
import <optional>;
import <thread>;

namespace ceassets = CR::Engine::Assets;
namespace ceaud    = CR::Engine::Audio;
namespace cecore   = CR::Engine::Core;
namespace ceinput  = CR::Engine::Input;
namespace cegraph  = CR::Engine::Graphics;
namespace ceplat   = CR::Engine::Platform;

namespace fs = std::filesystem;

using namespace std::literals;

int main(int, char*) {
	cecore::ServicesStart();

	fs::current_path(ceplat::GetCurrentProcessPath());

	cecore::LogSystem logSystem;

	bool done = false;
	ceplat::Window window("Conjure", 800, 600, [&done]() { done = true; });

	fs::path assetsPath = fs::canonical(ASSETS_FOLDER);

	cecore::AddService<ceassets::Service>(assetsPath);
	cecore::AddService<ceaud::Service>(false);
	cecore::AddService<ceinput::Service>(window);
	cecore::AddService<cegraph::Service>(window,
	                                     std::optional<glm::vec4>(std::in_place, 0.0f, 1.0f, 0.0f, 1.0f));

	auto& inputService    = cecore::GetService<ceinput::Service>();
	auto& graphicsService = cecore::GetService<cegraph::Service>();
	{
		auto handleFxs = ceaud::GetHandleFXs();
		auto music     = ceaud::GetHandleMusic();

		handleFxs.SetVolume(1.0f);
		music.SetVolume(0.75f);

		auto fanfareFX = ceaud::GetHandleFX(cecore::C_Hash64("levelupfanfare"));

		music.Play(cecore::C_Hash64("bgmMenu"));

		ceinput::Regions regions;
		auto region = regions.Create({{0, 0}, {400, 300}});

		while(!done) {
			window.UpdateInput();
			inputService.Update();

			if(regions.GetActive() == region && regions.WasActiveClicked()) { fanfareFX.Play(); }

			graphicsService.Update();
		}
	}

	cecore::ServicesStop();

	return EXIT_SUCCESS;
}