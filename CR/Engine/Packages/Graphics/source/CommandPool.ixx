module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.CommandPool;

import CR.Engine.Core;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Utils;

import std;
import std.compat;

export namespace CR::Engine::Graphics {
	namespace Handles {
		using CommandPool = CR::Engine::Core::Handle<class CommandPoolTag>;
	}

	// Secondary buffers aren't supported. they perform poorly on some mobile platforms.
	// Only resetting the entire pool is supported.
	namespace CommandPools {
		Handles::CommandPool Create(uint32_t a_queueFamily);
		void Delete(Handles::CommandPool a_pool);

		VkCommandBuffer Begin(Handles::CommandPool a_pool);
		void End(VkCommandBuffer a_buffer);
		void ResetAll(Handles::CommandPool a_pool);
	}    // namespace CommandPools

}    // namespace CR::Engine::Graphics

module :private;

namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	inline constexpr uint32_t c_maxPools = 16;

	struct PoolData {
		VkCommandPool commandPool{};
		std::vector<VkCommandBuffer> availableBuffers;
		std::vector<VkCommandBuffer> inUseBuffers;
	};

	cecore::HandlePool<cegraph::Handles::CommandPool, c_maxPools> m_handlePool;
	std::array<PoolData, c_maxPools> m_pools;

	void AllocateBuffers(cegraph::Handles::CommandPool a_pool) {
		constexpr uint32_t c_bufferGrowth = 2;

		VkCommandBufferAllocateInfo bufferInfo;
		cegraph::ClearStruct(bufferInfo);
		bufferInfo.commandBufferCount = c_bufferGrowth;
		bufferInfo.commandPool        = m_pools[a_pool].commandPool;
		bufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		auto& availBuffers       = m_pools[a_pool].availableBuffers;
		uint32_t newBufferOffset = (uint32_t)availBuffers.size();
		availBuffers.resize(availBuffers.size() + c_bufferGrowth);
		auto result = vkAllocateCommandBuffers(cegraph::GetContext().Device, &bufferInfo,
		                                       availBuffers.data() + newBufferOffset);
		CR_ASSERT(result == VK_SUCCESS, "Failed to allocate some vulkan command buffers");
	}

}    // namespace

cegraph::Handles::CommandPool cegraph::CommandPools::Create(uint32_t a_queueFamily) {
	CR_ASSERT(!m_handlePool.exhausted(), "Ran out of command pools");
	auto handle = m_handlePool.acquire();

	VkCommandPoolCreateInfo poolInfo;
	ClearStruct(poolInfo);
	poolInfo.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	poolInfo.queueFamilyIndex = a_queueFamily;
	auto result = vkCreateCommandPool(GetContext().Device, &poolInfo, nullptr, &m_pools[handle].commandPool);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create a vulkan command pool");

	AllocateBuffers(handle);

	return handle;
}

void cegraph::CommandPools::Delete(Handles::CommandPool a_pool) {
	CR_ASSERT(m_handlePool.isValid(a_pool), "Tried to delete an invalid pool");
	auto poolData = m_pools[a_pool];
	CR_ASSERT(poolData.inUseBuffers.empty(), "vulkan command buffers are still in use")
	// will free all command buffers as well
	vkDestroyCommandPool(GetContext().Device, poolData.commandPool, nullptr);

	m_handlePool.release(a_pool);
}

VkCommandBuffer cegraph::CommandPools::Begin(Handles::CommandPool a_pool) {
	CR_ASSERT(m_handlePool.isValid(a_pool), "Tried to delete an invalid pool");
	auto poolData = m_pools[a_pool];

	if(poolData.availableBuffers.empty()) { AllocateBuffers(a_pool); }

	auto buffer = poolData.availableBuffers.back();
	poolData.availableBuffers.pop_back();
	poolData.inUseBuffers.push_back(buffer);

	VkCommandBufferBeginInfo beginInfo;
	ClearStruct(beginInfo);
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(buffer, &beginInfo);

	return buffer;
}

void cegraph::CommandPools::End(VkCommandBuffer a_buffer) {
	auto result = vkEndCommandBuffer(a_buffer);
	CR_ASSERT(result == VK_SUCCESS, "Failed to end a vulkan command buffer");
}

void cegraph::CommandPools::ResetAll(Handles::CommandPool a_pool) {
	CR_ASSERT(m_handlePool.isValid(a_pool), "Tried to delete an invalid pool");
	auto poolData = m_pools[a_pool];

	auto result = vkResetCommandPool(GetContext().Device, poolData.commandPool, 0);
	CR_ASSERT(result == VK_SUCCESS, "Failed to reset a command pool");

	poolData.availableBuffers.insert(poolData.availableBuffers.end(), poolData.inUseBuffers.begin(),
	                                 poolData.inUseBuffers.end());
	poolData.inUseBuffers.clear();
}
