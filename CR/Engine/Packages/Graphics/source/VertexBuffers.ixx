module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.VertexBuffers;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.InternalHandles;
import CR.Engine.Graphics.VertexLayout;
import CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import std;
import std.compat;

export namespace CR::Engine::Graphics::VertexBuffers {
	void Initialize();
	void Shutdown();

	struct Mapping {
		// for passing to vulkan for your binding
		VkBuffer Buffer;
		uint32_t Size;

		// pointer to write your data too.
		// This may be write combine memory, so careful!
		std::byte* Data;
	};

	Handles::VertexBuffer Create(const VertexLayout& a_layout, uint32_t a_numVerts);
	void Release(Handles::VertexBuffer a_handle);

	Mapping Map(Handles::VertexBuffer a_handle);

	void Bind(Handles::VertexBuffer a_buffer, VkCommandBuffer& a_cmdBuffer);
}    // namespace CR::Engine::Graphics::VertexBuffers

module :private;

using namespace CR::Engine::Core::Literals;

namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	// way more than we need for a 2D engine.
	constexpr uint16_t c_maxVertexBuffers = 64;

	cecore::BitSet<c_maxVertexBuffers> m_used;
	cegraph::VertexBuffers::Mapping m_buffers[c_maxVertexBuffers];
	VmaAllocation m_allocations[c_maxVertexBuffers];

	VkVertexInputBindingDescription m_bindingDescriptions[c_maxVertexBuffers];
	std::vector<VkVertexInputAttributeDescription> m_attrDescriptions[c_maxVertexBuffers];
}    // namespace

void cegraph::VertexBuffers::Initialize() {}

void cegraph::VertexBuffers::Shutdown() {
	CR_ASSERT(m_used.empty(), "not all VertexBuffers were released prior to shutdown");
}

cegraph::Handles::VertexBuffer cegraph::VertexBuffers::Create(const VertexLayout& a_layout,
                                                              uint32_t a_numVerts) {
	Handles::VertexBuffer handle{m_used.FindNotInSet()};

	VkBufferCreateInfo bufferCreateInfo{};
	ClearStruct(bufferCreateInfo);
	bufferCreateInfo.size  = a_layout.GetSizeBytes() * a_numVerts;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo bufferAllocCreateInfo{};
	bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	bufferAllocCreateInfo.flags =
	    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	Mapping& mapping = m_buffers[handle];

	VmaAllocationInfo bufferAllocInfo{};
	vmaCreateBuffer(GetContext().Allocator, &bufferCreateInfo, &bufferAllocCreateInfo, &mapping.Buffer,
	                &(m_allocations[handle]), &bufferAllocInfo);
	mapping.Data = (std::byte*)bufferAllocInfo.pMappedData;
	mapping.Size = (uint32_t)bufferCreateInfo.size;

	// Only support instance vertex buffers at the moment. And binding would need to be changed in the
	// pipeline as appropriate, although only ever 1 binding at the moment.
	m_bindingDescriptions[handle].binding   = 0;
	m_bindingDescriptions[handle].stride    = a_layout.GetSizeBytes();
	m_bindingDescriptions[handle].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	m_attrDescriptions[handle].clear();
	for(const auto& entry : a_layout.GetLayout()) {
		VkVertexInputAttributeDescription desc = m_attrDescriptions[handle].emplace_back();

		desc.binding  = 0;
		desc.location = entry.Location;
		desc.offset   = entry.Offset;
		desc.format   = entry.format;
	}

	m_used.insert(handle);

	return handle;
}

void cegraph::VertexBuffers::Release(Handles::VertexBuffer a_handle) {
	CR_ASSERT(m_used.contains(a_handle), "Releasing VertexBuffers that doesn't exist");

	Mapping& mapping = m_buffers[a_handle];
	vmaDestroyBuffer(GetContext().Allocator, mapping.Buffer, m_allocations[a_handle]);
	m_used.erase(a_handle);
}

cegraph::VertexBuffers::Mapping cegraph::VertexBuffers::Map(Handles::VertexBuffer a_handle) {
	CR_ASSERT(m_used.contains(a_handle), "Mapping VertexBuffers that doesn't exist");

	return m_buffers[a_handle];
}

void cegraph::VertexBuffers::Bind(Handles::VertexBuffer a_buffer, VkCommandBuffer& a_cmdBuffer) {
	CR_ASSERT(m_used.contains(a_buffer), "Binding VertexBuffers that doesn't exist");

	VkDeviceSize vertOffset{};
	vkCmdBindVertexBuffers(a_cmdBuffer, 0, 1, &m_buffers[a_buffer].Buffer, &vertOffset);
}
