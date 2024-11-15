module;

#include <glm/glm.hpp>

export module CR.Engine.Graphics.Sprites;

import CR.Engine.Graphics.Handles;

import <cstdint>;
import <span>;

export namespace CR::Engine::Graphics::Sprites {
	void Create(std::span<uint64_t> a_hashes, std::span<Handles::Sprite> handles);
	void Delete(std::span<Handles::Sprite> a_sprites);

	void SetPositions(std::span<Handles::Sprite> a_sprites, std::span<glm::vec2> a_positions);
	// in radians
	void SetRotations(std::span<Handles::Sprite> a_sprites, std::span<float> a_rotations);
}    // namespace CR::Engine::Graphics::Sprites
