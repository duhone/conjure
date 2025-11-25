module;

#include <GLFW/glfw3.h>

export module CR.Engine;

export import CR.Engine.Core;
export import CR.Engine.Platform;
export import CR.Engine.Compression;
export import CR.Engine.Assets;
export import CR.Engine.Input;
export import CR.Engine.Audio;

export namespace CR::Engine {
	void Initialize(GLFWwindow* a_window, const std::filesystem::path& a_assetsFolder);
	void Update();
	void Render();
	void Shutdown();
}    // namespace CR::Engine

module :private;

namespace cecore   = CR::Engine::Core;
namespace ceplat   = CR::Engine::Platform;
namespace cecomp   = CR::Engine::Compression;
namespace ceassets = CR::Engine::Assets;
namespace ceinput  = CR::Engine::Input;
namespace ceaudio  = CR::Engine::Audio;

void CR::Engine::Initialize(GLFWwindow* a_window, const std::filesystem::path& a_assetsFolder) {
	cecore::Internal::Initialize();
	ceplat::Internal::Initialize();
	ceassets::Initialize(a_assetsFolder);
	ceinput::Initialize(a_window);
	ceaudio::Initialize();
}

void CR::Engine::Update() {
	ceplat::Internal::Update();
	ceinput::Update();
	ceaudio::Update();
}

void CR::Engine::Render() {}

void CR::Engine::Shutdown() {
	ceaudio::Shutdown();
	ceinput::Shutdown();
	ceassets::Shutdown();
	ceplat::Internal::Shutdown();
	cecore::Internal::Shutdown();
}
