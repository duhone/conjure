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

import <cstddef>;
import <vector>;

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
}    // namespace CR::Engine::Graphics::VertexBuffers

module :private;

using namespace CR::Engine::Core::Literals;

namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	// way more than we need for a 2D engine.
	constexpr uint16_t c_maxVertexBuffers = 64;

	struct Data {
		cecore::BitSet<c_maxVertexBuffers> Used;
		cegraph::VertexBuffers::Mapping Buffers[c_maxVertexBuffers];
		VmaAllocation Allocations[c_maxVertexBuffers];

		VkVertexInputBindingDescription m_bindingDescriptions[c_maxVertexBuffers];
		std::vector<VkVertexInputAttributeDescription> m_attrDescriptions[c_maxVertexBuffers];
	};

	Data* g_data = nullptr;
}    // namespace

void cegraph::VertexBuffers::Initialize() {
	CR_ASSERT(g_data == nullptr, "VertexBuffers are already initialized");

	g_data = new Data{};
}

void cegraph::VertexBuffers::Shutdown() {
	CR_ASSERT(g_data != nullptr, "VertexBuffers are already shutdown");
	CR_ASSERT(g_data->Used.empty(), "not all VertexBuffers were released prior to shutdown");

	delete g_data;
}

cegraph::Handles::VertexBuffer cegraph::VertexBuffers::Create(const VertexLayout& a_layout,
                                                              uint32_t a_numVerts) {
	CR_ASSERT(g_data != nullptr, "VertexBuffers are not initialized");

	Handles::VertexBuffer handle{g_data->Used.FindNotInSet()};

	VkBufferCreateInfo bufferCreateInfo{};
	ClearStruct(bufferCreateInfo);
	bufferCreateInfo.size  = a_layout.GetSizeBytes() * a_numVerts;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo bufferAllocCreateInfo{};
	bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	bufferAllocCreateInfo.flags =
	    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	Mapping& mapping = g_data->Buffers[handle.asInt()];

	VmaAllocationInfo bufferAllocInfo{};
	vmaCreateBuffer(GetContext().Allocator, &bufferCreateInfo, &bufferAllocCreateInfo, &mapping.Buffer,
	                &(g_data->Allocations[handle.asInt()]), &bufferAllocInfo);
	mapping.Data = (std::byte*)bufferAllocInfo.pMappedData;
	mapping.Size = bufferCreateInfo.size;

	// Only support instance vertex buffers at the moment. And binding would need to be changed in the
	// pipeline as appropriate, although only ever 1 binding at the moment.
	g_data->m_bindingDescriptions[handle.asInt()].binding   = 0;
	g_data->m_bindingDescriptions[handle.asInt()].stride    = a_layout.GetSizeBytes();
	g_data->m_bindingDescriptions[handle.asInt()].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	g_data->m_attrDescriptions[handle.asInt()].clear();
	for(const auto& entry : a_layout.GetLayout()) {
		VkVertexInputAttributeDescription desc = g_data->m_attrDescriptions[handle.asInt()].emplace_back();

		desc.binding  = 0;
		desc.location = entry.Location;
		desc.offset   = entry.Offset;
		desc.format   = entry.format;
	}

	g_data->Used.insert(handle.asInt());

	return handle;
}

void cegraph::VertexBuffers::Release(Handles::VertexBuffer a_handle) {
	CR_ASSERT(g_data != nullptr, "VertexBuffers are not initialized");
	CR_ASSERT(g_data->Used.contains(a_handle.asInt()), "Releasing VertexBuffers that doesn't exist");

	Mapping& mapping = g_data->Buffers[a_handle.asInt()];
	vmaDestroyBuffer(GetContext().Allocator, mapping.Buffer, g_data->Allocations[a_handle.asInt()]);
	g_data->Used.erase(a_handle.asInt());
}

cegraph::VertexBuffers::Mapping cegraph::VertexBuffers::Map(Handles::VertexBuffer a_handle) {
	CR_ASSERT(g_data != nullptr, "VertexBuffers are not initialized");
	CR_ASSERT(g_data->Used.contains(a_handle.asInt()), "Mapping VertexBuffers that doesn't exist");

	return g_data->Buffers[a_handle.asInt()];
}