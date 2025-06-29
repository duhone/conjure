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

		std::vector<uint64_t> textureSetHashes;
		textureSetHashes.emplace_back(cecore::C_Hash64("CompletionScreen"));
		textureSetHashes.emplace_back(cecore::C_Hash64("BonusHarrySelect"));
		textureSetHashes.emplace_back(cecore::C_Hash64("brick"));
		textureSetHashes.emplace_back(cecore::C_Hash64("diamond"));
		textureSetHashes.emplace_back(cecore::C_Hash64("gold"));
		textureSetHashes.emplace_back(cecore::C_Hash64("ice"));
		textureSetHashes.emplace_back(cecore::C_Hash64("leaf"));
		textureSetHashes.emplace_back(cecore::C_Hash64("m"));
		textureSetHashes.emplace_back(cecore::C_Hash64("question"));
		textureSetHashes.emplace_back(cecore::C_Hash64("wood"));
		auto textureSet = graphicsService.LoadTextureSet(textureSetHashes);

		std::vector<cegraph::Handles::Sprite> sprites;
		std::vector<glm::vec2> spritePositions;
		std::vector<float> spriteRotations;

		{
			std::vector<uint64_t> spriteHashes;
			spriteHashes.emplace_back(cecore::C_Hash64("brick"));
			sprites.resize(spriteHashes.size());
			cegraph::Sprites::Create(spriteHashes, sprites);
			spritePositions.emplace_back(glm::vec2{512.0f, 256.0f});
			cegraph::Sprites::SetPositions(sprites, spritePositions);
			spriteRotations.emplace_back(0.0f);
		}

		while(!done) {
			// Should really check for windows resize from OS as well. and minimized. the ReInitialize
			// graphics engine. Do that once we are using GLFW at top of loop. Not a problem so far on
			// windows, return value of graphics Update is taking care of it.

			window.UpdateInput();
			inputService.Update();

			spriteRotations[0] += 0.01f;
			cegraph::Sprites::SetRotations(sprites, spriteRotations);

			if(regions.GetActive() == region && regions.WasActiveClicked()) { fanfareFX.Play(); }

			bool gsAvailable = graphicsService.Update();
			if(!gsAvailable && !done) {
				gsAvailable = graphicsService.ReInitialize();
				if(!gsAvailable && !done) { std::this_thread::sleep_for(100ms); }
			}
		}

		cegraph::Sprites::Delete(sprites);
		graphicsService.ReleaseTextureSet(textureSet);
	}

	cecore::ServicesStop();

	return EXIT_SUCCESS;
}