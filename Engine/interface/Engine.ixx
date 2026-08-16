module;

#include <GLFW/glfw3.h>

export module CR.Engine;

export import CR.Engine.Core;
export import CR.Engine.Platform;
export import CR.Engine.Compression;
export import CR.Engine.Assets;
export import CR.Engine.Input;
export import CR.Engine.Audio;
export import CR.Engine.Graphics;

export namespace CR::Engine {
	void Initialize(GLFWwindow* a_window, const std::filesystem::path& a_assetsFolder);
	void Update();
	bool Render();
	void Shutdown();
}    // namespace CR::Engine

module :private;

namespace cecore   = CR::Engine::Core;
namespace ceplat   = CR::Engine::Platform;
namespace cecomp   = CR::Engine::Compression;
namespace ceassets = CR::Engine::Assets;
namespace ceinput  = CR::Engine::Input;
namespace ceaudio  = CR::Engine::Audio;
namespace cegraph  = CR::Engine::Graphics;

void CR::Engine::Initialize(GLFWwindow* a_window, const std::filesystem::path& a_gameAssetsFolder) {
	cecore::Internal::Initialize();
	ceplat::Internal::Initialize();
	ceassets::Initialize(a_gameAssetsFolder);
	ceinput::Initialize(a_window);
	ceaudio::Initialize();
	cegraph::Initialize(a_window);
}

void CR::Engine::Update() {
	ceplat::Internal::Update();
	ceinput::Update();
	ceaudio::Update();
	cegraph::Update();
}

bool CR::Engine::Render() {
	return cegraph::Render();
}

void CR::Engine::Shutdown() {
	cegraph::Shutdown();
	ceaudio::Shutdown();
	ceinput::Shutdown();
	ceassets::Shutdown();
	ceplat::Internal::Shutdown();
	cecore::Internal::Shutdown();
}
