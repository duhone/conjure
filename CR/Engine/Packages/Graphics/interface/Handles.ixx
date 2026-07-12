export module CR.Engine.Graphics.Handles;

import CR.Engine.Core;

export namespace CR::Engine::Graphics::Handles {
	using Texture    = CR::Engine::Core::Handle<class TextureHandleTag>;
	using TextureSet = CR::Engine::Core::Handle<class TextureSetHandleTag>;
	using Sprite     = CR::Engine::Core::Handle<class SpriteHandleTag>;
}    // namespace CR::Engine::Graphics::Handles
