export module CR.Engine.Graphics.InternalHandles;

import CR.Engine.Core;

export namespace CR::Engine::Graphics::Handles {
	using VertexBuffer  = CR::Engine::Core::Handle<class VertexBufferTag>;
	using Material      = CR::Engine::Core::Handle<class MaterialTag>;
	using DescriptorSet = CR::Engine::Core::Handle<class DescriptorSetTag>;
}    // namespace CR::Engine::Graphics::Handles
