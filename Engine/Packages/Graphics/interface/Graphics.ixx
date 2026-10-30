module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

export module CR.Engine.Graphics;

export import CR.Engine.Graphics.Handles;

import CR.Engine.Core.Span;

import std;
import std.compat;

export namespace CR::Engine::Graphics {
	// will be called from engine.
	void Initialize(GLFWwindow* a_window, glm::uvec2 a_designSize);
	void Update();
	bool Render();
	void Shutdown();

	bool ReInitialize();

	// should rarely need this. if covering entire screen with anything you won't need. set to nullopt to
	// disable if not already. default is disabled.
	void SetClearColor(std::optional<glm::vec4> a_clearColor);

	namespace Textures {
		extern "C++" Handles::Texture GetHandle(uint64_t hash);
		extern "C++" Handles::TextureSet LoadTextureSet(std::span<const uint64_t> hashes);
		extern "C++" void ReleaseTextureSet(Handles::TextureSet set);
	}    // namespace Textures

	namespace Sprites {
		// Everything here that takes SpanArg works on either a single value or a whole container/span, i.e.
		//   SetPositions(sprite, glm::vec2{10, 20});
		//   SetPositions(sprites, positions);
		// Note braced lists need the type named for single values, i.e. glm::vec2{10, 20} not just {10, 20}

		extern "C++" void Create(Core::SpanArg<const uint64_t> a_hashes, Core::SpanArg<Handles::Sprite> a_handles);
		Handles::Sprite Create(uint64_t a_hash) {
			Handles::Sprite handle;
			Create(a_hash, handle);
			return handle;
		}

		extern "C++" void Delete(Core::SpanArg<const Handles::Sprite> a_sprites);

		extern "C++" void GetSizes(Core::SpanArg<const Handles::Sprite> a_sprites,
		                           Core::SpanArg<glm::uvec2> a_sizes);
		glm::uvec2 GetSize(Handles::Sprite a_sprite) {
			glm::uvec2 size;
			GetSizes(a_sprite, size);
			return size;
		}
		// Can use this version to get size without creating a sprite, its a little bit slower though
		extern "C++" void GetSizes(Core::SpanArg<const uint64_t> a_spriteHashes,
		                           Core::SpanArg<glm::uvec2> a_sizes);
		glm::uvec2 GetSize(uint64_t a_spriteHash) {
			glm::uvec2 size;
			GetSizes(a_spriteHash, size);
			return size;
		}

		extern "C++" void SetPositions(Core::SpanArg<const Handles::Sprite> a_sprites,
		                              Core::SpanArg<const glm::vec2> a_positions);

		extern "C++" void SetZOrders(Core::SpanArg<const Handles::Sprite> a_sprites,
		                            Core::SpanArg<const uint8_t> a_zOrders);

		// in radians
		extern "C++" void SetRotations(Core::SpanArg<const Handles::Sprite> a_sprites,
		                              Core::SpanArg<const float> a_rotations);

		extern "C++" void SetFrames(Core::SpanArg<const Handles::Sprite> a_sprites,
		                           Core::SpanArg<const uint16_t> a_frames);

		extern "C++" void SetColors(Core::SpanArg<const Handles::Sprite> a_sprites,
		                           Core::SpanArg<const glm::u8vec4> a_colors);

		extern "C++" void SetVisibilities(Core::SpanArg<const Handles::Sprite> a_sprites,
		                                Core::SpanArg<const bool> a_visibilities);

		extern "C++" void SetAnimationPauses(Core::SpanArg<const Handles::Sprite> a_sprites,
		                                     Core::SpanArg<const bool> a_pauses);

		// will fail if any of the dest sprites aren't bassed on the same template as the source sprite.
		extern "C++" void SyncAnimations(Handles::Sprite a_sourceSprite,
		                                Core::SpanArg<const Handles::Sprite> a_destSprites);
	}    // namespace Sprites
}    // namespace CR::Engine::Graphics
