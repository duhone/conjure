#include <engine/Engine.h>

#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>

import CR.Engine;

import std;
import std.compat;

// namespace ceassets = CR::Engine::Assets;
//  namespace ceaud    = CR::Engine::Audio;
namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;
// namespace cegraph  = CR::Engine::Graphics;
namespace ceplat = CR::Engine::Platform;

namespace fs = std::filesystem;

using namespace std::literals;

namespace {
	void glfwErrorCallback(int error, const char* description) {
		CR_WARN("GLFW Error {}: {}", error, description);
	}
}    // namespace

int main(int, char*) {
	fs::current_path(ceplat::GetCurrentProcessPath());

	cecore::LogSystem logSystem;

	fs::path assetsPath = fs::canonical(ASSETS_FOLDER);

	glfwSetErrorCallback(glfwErrorCallback);

	if(!glfwInit()) {
		CR_ASSERT_ALWAYS(false, "failed to initialize glfw");
		return 0;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Conjure", NULL, NULL);
	if(!window) {
		CR_ASSERT_ALWAYS(false, "failed to create glfw window");
		return 0;
	}

	CR::Engine::Initialize(window, assetsPath);

	// auto handleFxs = ceaud::GetHandleFXs();
	// auto music     = ceaud::GetHandleMusic();

	// handleFxs.SetVolume(1.0f);
	// music.SetVolume(0.75f);

	// auto fanfareFX = ceaud::GetHandleFX(cecore::C_Hash64("levelupfanfare"));

	// music.Play(cecore::C_Hash64("bgmMenu"));

	ceinput::Handles::Region region = ceinput::Regions::create({{0, 0}, {400, 300}});

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
	// auto textureSet = graphicsService.LoadTextureSet(textureSetHashes);

	// std::vector<cegraph::Handles::Sprite> sprites;
	// std::vector<glm::vec2> spritePositions;
	// std::vector<float> spriteRotations;

	// constexpr uint32_t numSprites = 64;
	{
		/*std::vector<uint64_t> spriteHashes;
		for(uint32_t i = 0; i < numSprites; ++i) {
		    spriteHashes.emplace_back(
		        textureSetHashes[cecore::Random(2, (int32_t)textureSetHashes.size() - 1)]);

		    spritePositions.emplace_back(
		        glm::vec2{cecore::Random(0.0f, 700.0f), cecore::Random(0.0f, 400.0f)});
		    spriteRotations.emplace_back(cecore::Random(0.0f, 3.14f));
		}

		spriteHashes.emplace_back(cecore::C_Hash64("CompletionScreen"));
		spritePositions.emplace_back(glm::vec2{400.0f, 300.0f});
		spriteRotations.emplace_back(0.0f);

		spriteHashes.emplace_back(cecore::C_Hash64("BonusHarrySelect"));
		spritePositions.emplace_back(glm::vec2{100.0f, 100.0f});
		spriteRotations.emplace_back(0.0f);

		sprites.resize(spriteHashes.size());
		cegraph::Sprites::Create(spriteHashes, sprites);
		cegraph::Sprites::SetPositions(sprites, spritePositions);
		cegraph::Sprites::SetRotations(sprites, spriteRotations);*/
	}

	// std::vector<float> spriteRotSpeeds;
	// for(uint32_t i = 0; i < numSprites; ++i) {
	//	spriteRotSpeeds.emplace_back(cecore::Random(0.005f, 0.05f));
	// }

	uint32_t frameCount = 0;
	auto startFPSTime   = std::chrono::high_resolution_clock::now();
	while(!glfwWindowShouldClose(window)) {
		// Should really check for windows resize from OS as well. and minimized. the ReInitialize
		// graphics engine. Do that once we are using GLFW at top of loop. Not a problem so far on
		// windows, return value of graphics Update is taking care of it.

		glfwPollEvents();

		CR::Engine::Update();

		// for(uint32_t i = 0; i < numSprites; ++i) { spriteRotations[i] += spriteRotSpeeds[i]; }
		// cegraph::Sprites::SetRotations(sprites, spriteRotations);

		uint32_t regionState;
		ceinput::Regions::getStates({&region, 1}, {&regionState, 1});

		if((regionState & ceinput::Regions::RegionStates::Pressed) != 0) {
			// fanfareFX.Play();
		}

		CR::Engine::Render();

		/*bool gsAvailable = graphicsService.Update();
		if(!gsAvailable && !done) {
		    gsAvailable = graphicsService.ReInitialize();
		    if(!gsAvailable && !done) { std::this_thread::sleep_for(100ms); }
		}
		*/

		++frameCount;
		if(frameCount == 1024) {
			frameCount      = 0;
			auto endFPSTime = std::chrono::high_resolution_clock::now();
			double times =
			    std::chrono::duration_cast<std::chrono::milliseconds>(endFPSTime - startFPSTime).count() /
			    1000.0f;
			startFPSTime = endFPSTime;
			CR_LOG("FPS {:.2f}", (1024.0f / times));
		}
	}

	// cegraph::Sprites::Delete(sprites);
	// graphicsService.ReleaseTextureSet(textureSet);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 1;
}