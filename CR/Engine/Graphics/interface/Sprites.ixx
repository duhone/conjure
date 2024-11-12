module;

#include <glm/glm.hpp>

export module CR.Engine.Graphics.Sprites;

import CR.Engine.Graphics.Handles;

import <cstdint>;
import <span>;

export namespace CR::Engine::Graphics::Sprites {
	Handles::Sprite Create(uint64_t a_hash);
	void Delete(Handles::Sprite a_sprite);

	void SetPositions(std::span<Handles::Sprite> a_sprites, std::span<glm::vec2> a_positions);
	// in radians
	void SetRotations(std::span<Handles::Sprite> a_sprites, std::span<float> a_rotations);
}    // namespace CR::Engine::Graphics::Sprites
