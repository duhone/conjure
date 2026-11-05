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
		extern "C++" void Create(Core::SpanArg<const uint64_t> a_hashes,
		                         Core::SpanArg<Handles::Sprite> a_handles);

		extern "C++" void Delete(Core::SpanArg<const Handles::Sprite> a_sprites);

		extern "C++" void GetSizes(Core::SpanArg<const Handles::Sprite> a_sprites,
		                           Core::SpanArg<glm::uvec2> a_sizes);
		// Can use this version to get size without creating a sprite, its a little bit slower though
		extern "C++" void GetSizes(Core::SpanArg<const uint64_t> a_spriteHashes,
		                           Core::SpanArg<glm::uvec2> a_sizes);

		extern "C++" void GetCurrentFrames(Core::SpanArg<const Handles::Sprite> a_sprites,
		                                   Core::SpanArg<uint16_t> a_frames);

		// always false for looping animations, true for non looping animations that have reached the end.
		extern "C++" void AreAnimationsDone(Core::SpanArg<const Handles::Sprite> a_sprites,
		                                    Core::SpanArg<bool> a_done);

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

		// only has meaning for non looping animations. will set frame back to 0, and start animating.
		extern "C++" void StartAnimations(Core::SpanArg<const Handles::Sprite> a_sprites);
	}    // namespace Sprites
}    // namespace CR::Engine::Graphics
