module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.UniformBuffer;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import <cstddef>;
import <vector>;

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

	struct Data {
		// one buffer per frame in flight.
		VkBuffer Buffer;
		VmaAllocation Allocation;
		std::byte* DataPtr;

		std::byte* CurrentData{};
	};

	Data* g_data = nullptr;
}    // namespace

void cegraph::UniformBuffer::Initialize() {
	CR_ASSERT(g_data == nullptr, "Textures are already initialized");
	g_data = new Data{};

	VkBufferCreateInfo bufferCreateInfo{};
	ClearStruct(bufferCreateInfo);
	bufferCreateInfo.size  = c_uniformBufferSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	VmaAllocationCreateInfo bufferAllocCreateInfo{};
	bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	bufferAllocCreateInfo.flags =
	    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo bufferAllocInfo{};
	vmaCreateBuffer(GetContext().Allocator, &bufferCreateInfo, &bufferAllocCreateInfo, &g_data->Buffer,
	                &g_data->Allocation, &bufferAllocInfo);
	g_data->DataPtr = (std::byte*)bufferAllocInfo.pMappedData;
}

void cegraph::UniformBuffer::Shutdown() {
	CR_ASSERT(g_data != nullptr, "UniformBuffer is already shutdown");

	vmaDestroyBuffer(GetContext().Allocator, g_data->Buffer, g_data->Allocation);

	delete g_data;
}

void cegraph::UniformBuffer::Update() {
	CR_ASSERT(g_data != nullptr, "UniformBuffer is not initialized");

	g_data->CurrentData = g_data->DataPtr;
}

cegraph::UniformBuffer::Mapping cegraph::UniformBuffer::Map(uint32_t a_size) {
	CR_ASSERT(g_data != nullptr, "UniformBuffer is not initialized");

	// always start at a 256 byte alignment
	g_data->CurrentData =
	    reinterpret_cast<std::byte*>((reinterpret_cast<uint64_t>(g_data->CurrentData) + 255ull) & (~255ull));

	Mapping result{};
	result.Buffer = g_data->Buffer;
	result.Offset = uint32_t(g_data->CurrentData - g_data->DataPtr);
	result.Data   = g_data->CurrentData;
	result.Size   = a_size;

	g_data->CurrentData += a_size;

	return result;
}
