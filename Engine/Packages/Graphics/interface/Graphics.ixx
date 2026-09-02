module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

export module CR.Engine.Graphics;

export import CR.Engine.Graphics.Handles;

import std;
import std.compat;

export namespace CR::Engine::Graphics {
	// will be called from engine.
	void Initialize(GLFWwindow* a_window);
	void Update();
	bool Render();
	void Shutdown();

	bool ReInitialize();

	// should rarely need this. if covering entire screen with anything you won't need. set to nullopt to
	// disable if not already. default is disabled.
	void SetClearColor(std::optional<glm::vec4> a_clearColor);

	namespace Textures {
		extern "C++" Handles::Texture GetHandle(uint64_t hash);
		extern "C++" Handles::TextureSet LoadTextureSet(std::span<uint64_t> hashes);
		extern "C++" void ReleaseTextureSet(Handles::TextureSet set);
	}    // namespace Textures

	namespace Sprites {
		extern "C++" void Create(std::span<uint64_t> a_hashes, std::span<Handles::Sprite> handles);
		Handles::Sprite Create(uint64_t a_hash) {
			Handles::Sprite handle;
			Create(std::span<uint64_t>(&a_hash, 1), std::span<Handles::Sprite>(&handle, 1));
			return handle;
		}

		extern "C++" void Delete(std::span<Handles::Sprite> a_sprites);
		void Delete(Handles::Sprite a_sprite) {
			Delete(std::span<Handles::Sprite>(&a_sprite, 1));
		}

		extern "C++" void SetPositions(std::span<Handles::Sprite> a_sprites,
		                               std::span<glm::vec2> a_positions);
		void SetPosition(Handles::Sprite a_sprite, glm::vec2 a_position) {
			SetPositions(std::span<Handles::Sprite>(&a_sprite, 1), std::span<glm::vec2>(&a_position, 1));
		}

		// in radians
		extern "C++" void SetRotations(std::span<Handles::Sprite> a_sprites, std::span<float> a_rotations);
		void SetRotation(Handles::Sprite a_sprite, float a_rotation) {
			SetRotations(std::span<Handles::Sprite>(&a_sprite, 1), std::span<float>(&a_rotation, 1));
		}

		extern "C++" void SetFrame(std::span<Handles::Sprite> a_sprites, std::span<uint16_t> a_frames);
		void SetFrame(Handles::Sprite a_sprite, uint16_t a_frame) {
			SetFrame(std::span<Handles::Sprite>(&a_sprite, 1), std::span<uint16_t>(&a_frame, 1));
		}
	}    // namespace Sprites
}    // namespace CR::Engine::Graphics
