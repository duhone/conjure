module;

#include "core/Log.h"

#include "volk.h"

export module CR.Engine.Graphics.CommandPool;

import CR.Engine.Core;
import CR.Engine.Graphics.Utils;

import <vector>;

namespace CR::Engine::Graphics {
	struct CommandBuffer : public CR::Engine::Core::Handle<CommandBuffer> {
		using CR::Engine::Core::Handle<CommandBuffer>::Handle;
	};

	// Secondary buffers aren't supported. they perform poorly on some mobile platforms.
	// Only resetting the entire pool is supported.
	class CommandPool {
	  public:
		CommandPool() = default;
		CommandPool(VkDevice a_device, uint32_t a_queueFamily);
		~CommandPool();
		CommandPool(const CommandPool&)               = delete;
		CommandPool(CommandPool&& a_other)            = delete;
		CommandPool& operator=(const CommandPool&)    = delete;
		CommandPool& operator=(CommandPool&& a_other) = delete;

		[[nodiscard]] VkCommandBuffer Begin();
		void End(VkCommandBuffer a_buffer);

		void ResetAll();

	  private:
		void AllocateBuffers();

		VkDevice m_device;
		VkCommandPool m_commandPool{};
		std::vector<VkCommandBuffer> m_availableBuffers;
		std::vector<VkCommandBuffer> m_inUseBuffers;
	};

}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

cegraph::CommandPool::CommandPool(VkDevice a_device, uint32_t a_queueFamily) : m_device(a_device) {
	VkCommandPoolCreateInfo poolInfo;
	ClearStruct(poolInfo);
	poolInfo.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	poolInfo.queueFamilyIndex = a_queueFamily;
	auto result               = vkCreateCommandPool(a_device, &poolInfo, nullptr, &m_commandPool);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create a vulkan command pool");

	AllocateBuffers();
}

cegraph::CommandPool::~CommandPool() {
	if(m_commandPool) {
		CR_ASSERT(m_inUseBuffers.empty(), "vulkan command buffers are still in use")
		// will free all command buffers as well
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	}
}

void cegraph::CommandPool::AllocateBuffers() {
	constexpr uint32_t c_bufferGrowth = 2;

	VkCommandBufferAllocateInfo bufferInfo;
	ClearStruct(bufferInfo);
	bufferInfo.commandBufferCount = c_bufferGrowth;
	bufferInfo.commandPool        = m_commandPool;
	bufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	uint32_t newBufferOffset = m_availableBuffers.size();
	m_availableBuffers.resize(m_availableBuffers.size() + c_bufferGrowth);
	auto result =
	    vkAllocateCommandBuffers(m_device, &bufferInfo, m_availableBuffers.data() + newBufferOffset);
	CR_ASSERT(result == VK_SUCCESS, "Failed to allocate some vulkan command buffers");
}

VkCommandBuffer cegraph::CommandPool::Begin() {
	if(m_availableBuffers.empty()) { AllocateBuffers(); }

	auto buffer = m_availableBuffers.back();
	m_availableBuffers.pop_back();
	m_inUseBuffers.push_back(buffer);

	VkCommandBufferBeginInfo beginInfo;
	ClearStruct(beginInfo);
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(buffer, &beginInfo);

	return buffer;
}

void cegraph::CommandPool::End(VkCommandBuffer a_buffer) {
	auto result = vkEndCommandBuffer(a_buffer);
	CR_ASSERT(result == VK_SUCCESS, "Failed to end a vulkan command buffer");
}

void cegraph::CommandPool::ResetAll() {
	auto result = vkResetCommandPool(m_device, m_commandPool, 0);
	CR_ASSERT(result == VK_SUCCESS, "Failed to reset a command pool");

	m_availableBuffers.insert(m_availableBuffers.end(), m_inUseBuffers.begin(), m_inUseBuffers.end());
	m_inUseBuffers.clear();
}
