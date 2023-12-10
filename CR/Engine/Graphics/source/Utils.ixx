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
		} else if constexpr(std::is_same_v<T, VkShaderModuleCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkPipelineShaderStageCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkPipelineViewportStateCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkPipelineDynamicStateCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkPipelineRasterizationStateCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkPipelineMultisampleStateCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkPipelineColorBlendStateCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkSamplerCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkDescriptorSetLayoutCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkPipelineLayoutCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkGraphicsPipelineCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkPipelineInputAssemblyStateCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkPipelineVertexInputStateCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		} else if constexpr(std::is_same_v<T, VkComputePipelineCreateInfo>) {
			value.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		}
	}

	export template<typename T, int SIZE>
	inline void ClearStruct(T a_values[SIZE]) {
		for(const T& val : a_values) { ClearStruct(val); }
	}

}    // namespace CR::Engine::Graphics