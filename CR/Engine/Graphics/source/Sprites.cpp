module CR.Engine.Graphics.Sprites;

import CR.Engine.Graphics.SpritesInternal;

namespace cegraph = CR::Engine::Graphics;

cegraph::Handles::Sprite cegraph::Sprites::Create() {
	return cegraph::Sprites::CreateInternal();
}

void cegraph::Sprites::Delete(cegraph::Handles::Sprite a_sprite) {
	cegraph::Sprites::Delete(a_sprite);
}
