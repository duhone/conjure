module;

#include <glm/glm.hpp>

module CR.Engine.Graphics.Sprites;

import CR.Engine.Graphics.SpritesInternal;

namespace cegraph = CR::Engine::Graphics;

cegraph::Handles::Sprite cegraph::Sprites::Create(uint64_t a_hash) {
	return cegraph::Sprites::CreateInternal(a_hash);
}

void cegraph::Sprites::Delete(cegraph::Handles::Sprite a_sprite) {
	cegraph::Sprites::DeleteInternal(a_sprite);
}

void cegraph::Sprites::SetPositions(std::span<Handles::Sprite> a_sprites, std::span<glm::vec2> a_positions) {
	cegraph::Sprites::SetPositionsInternal(a_sprites, a_positions);
}

void cegraph::Sprites::SetRotations(std::span<Handles::Sprite> a_sprites, std::span<float> a_rotations) {
	cegraph::Sprites::SetRotationsInternal(a_sprites, a_rotations);
}
