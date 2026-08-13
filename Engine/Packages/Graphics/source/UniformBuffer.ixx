module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.UniformBuffer;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import std;
import std.compat;

export namespace CR::Engine::Graphics::UniformBuffer {
	void Initialize();
	void Update();
	void Shutdown();

	struct Mapping {
		// for passing to vulkan for your binding
		VkBuffer Buffer;
		uint32_t Offset;
		uint32_t Size;

		// pointer to write your data too. points to beginning, don't need the above offset.
		// This may be write combine memory, so careful!
		std::byte* Data;
	};

	// no need to unmap or do any release. result only valid this frame.
	Mapping Map(uint32_t a_size);
}    // namespace CR::Engine::Graphics::UniformBuffer

module :private;

using namespace CR::Engine::Core::Literals;

namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	// if need more, not a big deal to increase this.
	constexpr uint64_t c_uniformBufferSize = 4_MB;

	// one buffer per frame in flight.
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	std::byte* m_dataPtr;

	std::byte* m_currentData{};
}    // namespace

void cegraph::UniformBuffer::Initialize() {
	VkBufferCreateInfo bufferCreateInfo{};
	ClearStruct(bufferCreateInfo);
	bufferCreateInfo.size  = c_uniformBufferSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VmaAllocationCreateInfo bufferAllocCreateInfo{};
	bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	bufferAllocCreateInfo.flags =
	    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo bufferAllocInfo{};
	vmaCreateBuffer(GetContext().Allocator, &bufferCreateInfo, &bufferAllocCreateInfo, &m_buffer,
	                &m_allocation, &bufferAllocInfo);
	m_dataPtr = (std::byte*)bufferAllocInfo.pMappedData;
}

void cegraph::UniformBuffer::Shutdown() {
	vmaDestroyBuffer(GetContext().Allocator, m_buffer, m_allocation);
}

void cegraph::UniformBuffer::Update() {
	m_currentData = m_dataPtr;
}

cegraph::UniformBuffer::Mapping cegraph::UniformBuffer::Map(uint32_t a_size) {
	// always start at a 256 byte alignment
	m_currentData =
	    reinterpret_cast<std::byte*>((reinterpret_cast<uint64_t>(m_currentData) + 255ull) & (~255ull));

	Mapping result{};
	result.Buffer = m_buffer;
	result.Offset = uint32_t(m_currentData - m_dataPtr);
	result.Data   = m_currentData;
	result.Size   = a_size;

	m_currentData += a_size;

	return result;
}
