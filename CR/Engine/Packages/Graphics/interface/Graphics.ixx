module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

export module CR.Engine.Graphics;

export import CR.Engine.Graphics.Handles;

import std;
import std.compat;

export namespace CR::Engine::Graphics {
	void Initialize(GLFWwindow* a_window);
	void Update();
	void Render();
	void Shutdown();

	namespace Textures {
		extern "C++" Handles::Texture GetHandle(uint64_t hash);
		extern "C++" Handles::TextureSet LoadTextureSet(std::span<uint64_t> hashes);
		extern "C++" void ReleaseTextureSet(Handles::TextureSet set);
	}    // namespace Textures

	namespace Sprites {
		extern "C++" void Create(std::span<uint64_t> a_hashes, std::span<Handles::Sprite> handles);
		extern "C++" void Delete(std::span<Handles::Sprite> a_sprites);

		extern "C++" void SetPositions(std::span<Handles::Sprite> a_sprites,
		                               std::span<glm::vec2> a_positions);
		// in radians
		extern "C++" void SetRotations(std::span<Handles::Sprite> a_sprites, std::span<float> a_rotations);
	}    // namespace Sprites
}    // namespace CR::Engine::Graphics
