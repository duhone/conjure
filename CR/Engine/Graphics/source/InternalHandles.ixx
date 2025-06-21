export module CR.Engine.Graphics.InternalHandles;

import CR.Engine.Core;

namespace CR::Engine::Graphics::Handles {
	export using VertexBuffer  = CR::Engine::Core::Handle<class VertexBufferTag>;
	export using Material      = CR::Engine::Core::Handle<class MaterialTag>;
	export using DescriptorSet = CR::Engine::Core::Handle<class DescriptorSetTag>;
}    // namespace CR::Engine::Graphics::Handles
