module;

#include <glm/glm.hpp>

module CR.Engine.Graphics.Sprites;

import CR.Engine.Graphics.SpritesInternal;

namespace cegraph = CR::Engine::Graphics;

void cegraph::Sprites::Create(std::span<uint64_t> a_hashes, std::span<Handles::Sprite> handles) {
	cegraph::Sprites::CreateInternal(a_hashes, handles);
}

void cegraph::Sprites::Delete(std::span<Handles::Sprite> a_sprites) {
	cegraph::Sprites::DeleteInternal(a_sprites);
}

void cegraph::Sprites::SetPositions(std::span<Handles::Sprite> a_sprites, std::span<glm::vec2> a_positions) {
	cegraph::Sprites::SetPositionsInternal(a_sprites, a_positions);
}

void cegraph::Sprites::SetRotations(std::span<Handles::Sprite> a_sprites, std::span<float> a_rotations) {
	cegraph::Sprites::SetRotationsInternal(a_sprites, a_rotations);
}
