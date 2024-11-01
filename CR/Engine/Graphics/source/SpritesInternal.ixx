export module CR.Engine.Graphics.SpritesInternal;

import CR.Engine.Graphics.Handles;

export namespace CR::Engine::Graphics::Sprites {
	// public API
	Handles::Sprite CreateInternal();
	void DeleteInternal(Handles::Sprite a_sprite);

	void Initialize();
	void Shutdown();

}    // namespace CR::Engine::Graphics::Sprites

module :private;

namespace cegraph = CR::Engine::Graphics;

cegraph::Handles::Sprite cegraph::Sprites::CreateInternal() {
	return cegraph::Handles::Sprite{};
}

void cegraph::Sprites::DeleteInternal(cegraph::Handles::Sprite a_sprite) {}

void cegraph::Sprites::Initialize() {}

void cegraph::Sprites::Shutdown() {}
