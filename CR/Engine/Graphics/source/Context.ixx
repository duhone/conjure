module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.Context;

namespace CR::Engine::Graphics {
	export struct Context {
		VkPhysicalDevice PhysicalDevice;
		VkDevice Device;
		VmaAllocator Allocator;
	};
}    // namespace CR::Engine::Graphics
