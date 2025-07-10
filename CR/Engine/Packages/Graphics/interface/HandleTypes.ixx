export module CR.Engine.Graphics.Handles;

import CR.Engine.Core;

namespace CR::Engine::Graphics::Handles {
	export using Texture    = CR::Engine::Core::Handle<class TextureHandleTag>;
	export using TextureSet = CR::Engine::Core::Handle<class TextureSetHandleTag>;
	export using Sprite     = CR::Engine::Core::Handle<class SpriteHandleTag>;
}    // namespace CR::Engine::Graphics::Handles
