module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.MultiDrawBuffer;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import std;
import std.compat;

// Currently setup for CPU based multidraw
export namespace CR::Engine::Graphics::MultiDrawBuffer {
	void Initialize();
	void Update();
	void Shutdown();

	struct Mapping {
		// for passing to vulkan for your binding
		VkBuffer Buffer;
		uint32_t Offset;
		uint32_t Size;

		// pointer to write your commands too. points to beginning, don't need the above offset.
		// This may be write combine memory, so careful!
		VkDrawIndirectCommand* Data;
	};

	// no need to unmap or do any release. result only valid this frame.
	Mapping Map(uint32_t a_numCommands);
}    // namespace CR::Engine::Graphics::MultiDrawBuffer

module :private;

using namespace CR::Engine::Core::Literals;

namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	// if need more, not a big deal to increase this. 64K draw commands is a lot though.
	constexpr uint64_t c_multiDrawBufferSize = sizeof(VkDrawIndirectCommand) * 64_KB;

	// one buffer , only one frame in flight.
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	VkDrawIndirectCommand* m_dataPtr{};

	VkDrawIndirectCommand* m_currentCommand{};

}    // namespace

void cegraph::MultiDrawBuffer::Initialize() {
	CR_ASSERT(m_dataPtr == nullptr, "MultiDrawBuffer are already initialized");

	VkBufferCreateInfo bufferCreateInfo{};
	ClearStruct(bufferCreateInfo);
	bufferCreateInfo.size  = c_multiDrawBufferSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

	VmaAllocationCreateInfo bufferAllocCreateInfo{};
	bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	bufferAllocCreateInfo.flags =
	    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo bufferAllocInfo{};
	vmaCreateBuffer(GetContext().Allocator, &bufferCreateInfo, &bufferAllocCreateInfo, &m_buffer,
	                &m_allocation, &bufferAllocInfo);
	m_dataPtr = (VkDrawIndirectCommand*)bufferAllocInfo.pMappedData;
}

void cegraph::MultiDrawBuffer::Shutdown() {
	CR_ASSERT(m_dataPtr != nullptr, "MultiDrawBuffer is already shutdown");

	vmaDestroyBuffer(GetContext().Allocator, m_buffer, m_allocation);

	m_dataPtr = nullptr;
}

void cegraph::MultiDrawBuffer::Update() {
	CR_ASSERT(m_dataPtr != nullptr, "MultiDrawBuffer is not initialized");

	m_currentCommand = m_dataPtr;
}

cegraph::MultiDrawBuffer::Mapping cegraph::MultiDrawBuffer::Map(uint32_t a_numCommands) {
	CR_ASSERT(m_dataPtr != nullptr, "MultiDrawBuffer is not initialized");

	Mapping result{};
	result.Buffer = m_buffer;
	result.Offset = uint32_t((std::byte*)m_currentCommand - (std::byte*)m_dataPtr);
	result.Data   = m_currentCommand;
	result.Size   = a_numCommands * sizeof(VkDrawIndirectCommand);

	m_currentCommand += a_numCommands;

	return result;
}
