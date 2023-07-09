module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import <cstring>;
import <type_traits>;

namespace CR::Engine::Graphics {
	export template<typename T>
	inline void ClearStruct(T& value) {
		memset(&value, 0, sizeof(T));
		if constexpr(std::is_same_v<T, VkApplicationInfo>) {
			value.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		} else if constexpr(std::is_same_v<T, VkInstanceCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkWin32SurfaceCreateInfoKHR>) {
			value.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		} else if constexpr(std::is_same_v<T, VkPhysicalDeviceVulkan12Features>) {
			value.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		} else if constexpr(std::is_same_v<T, VkPhysicalDeviceVulkan11Features>) {
			value.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		} else if constexpr(std::is_same_v<T, VkPhysicalDeviceFeatures2>) {
			value.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		} else if constexpr(std::is_same_v<T, VkDeviceQueueCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkDeviceCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkImageCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkMemoryAllocateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		} else if constexpr(std::is_same_v<T, VkImageViewCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkSwapchainCreateInfoKHR>) {
			value.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		} else if constexpr(std::is_same_v<T, VkAttachmentDescription>) {
			// no sType on this one
		} else if constexpr(std::is_same_v<T, VkAttachmentReference>) {
			// no sType on this one
		} else if constexpr(std::is_same_v<T, VkSubpassDescription>) {
			// no sType on this one
		} else if constexpr(std::is_same_v<T, VkRenderPassCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkFramebufferCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkSemaphoreCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkFenceCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkSubmitInfo>) {
			value.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		} else if constexpr(std::is_same_v<T, VkPresentInfoKHR>) {
			value.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		} else if constexpr(std::is_same_v<T, VkCommandPoolCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkCommandBufferAllocateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		} else if constexpr(std::is_same_v<T, VkCommandBufferBeginInfo>) {
			value.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		} else if constexpr(std::is_same_v<T, VkRenderPassBeginInfo>) {
			value.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		} else {
			static_assert(Core::always_false_v<T>, "unsuported Vulkan struct type in ClearStruct");
		}
	}
}    // namespace CR::Engine::Graphics