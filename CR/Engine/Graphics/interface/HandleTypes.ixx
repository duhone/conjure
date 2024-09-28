export module CR.Engine.Graphics.Handles;

import CR.Engine.Core;

namespace CR::Engine::Graphics::Handles {
	export using Texture    = CR::Engine::Core::Handle<class TextureHandleTag>;
	export using TextureSet = CR::Engine::Core::Handle<class TextureSetHandleTag>;
}    // namespace CR::Engine::Graphics::Handles
