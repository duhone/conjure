module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.Context;

namespace CR::Engine::Graphics {
	export struct Context {
		VkPhysicalDevice PhysicalDevice;
		VkDevice Device;
		VmaAllocator Allocator;
		int32_t GraphicsQueueIndex{-1};
		int32_t TransferQueueIndex{-1};
	};

	export const Context& GetContext();
	export void SetContext(const Context& a_context);
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

namespace {
	cegraph::Context g_context;
}

const cegraph::Context& cegraph::GetContext() {
	return g_context;
}
void cegraph::SetContext(const Context& a_context) {
	g_context = a_context;
}
