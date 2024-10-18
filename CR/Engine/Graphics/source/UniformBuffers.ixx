module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.UniformBuffers;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import <cstddef>;
import <vector>;

export namespace CR::Engine::Graphics::UniformBuffers {
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
	Mapping Map(uint32_t a_initialSize);
	// add more space to previous map, only valid for appending to the last call to Map.
	void Append(Mapping& a_map, uint32_t a_additionalSize);
}    // namespace CR::Engine::Graphics::UniformBuffers

module :private;

using namespace CR::Engine::Core::Literals;

namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	// if need more, not a big deal to increase this.
	constexpr uint64_t c_uniformBufferSize = 4_MB;

	struct Data {
		// one buffer per frame in flight.
		VkBuffer Buffers[cegraph::Constants::c_maxFramesInFlight];
		VmaAllocation Allocations[cegraph::Constants::c_maxFramesInFlight];
		std::byte* Data[cegraph::Constants::c_maxFramesInFlight];

		uint32_t CurrentFrame{};
		std::byte* CurrentData{};
	};

	Data* g_data = nullptr;
}    // namespace

void cegraph::UniformBuffers::Initialize() {
	CR_ASSERT(g_data == nullptr, "Textures are already initialized");
	g_data = new Data{};

	for(int32_t i = 0; i < cegraph::Constants::c_maxFramesInFlight; ++i) {
		VkBufferCreateInfo bufferCreateInfo{};
		ClearStruct(bufferCreateInfo);
		bufferCreateInfo.size  = c_uniformBufferSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		VmaAllocationCreateInfo bufferAllocCreateInfo{};
		bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
		bufferAllocCreateInfo.flags =
		    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo bufferAllocInfo{};
		vmaCreateBuffer(GetContext().Allocator, &bufferCreateInfo, &bufferAllocCreateInfo,
		                &g_data->Buffers[i], &g_data->Allocations[i], &bufferAllocInfo);
		g_data->Data[i] = (std::byte*)bufferAllocInfo.pMappedData;
	}
}

void cegraph::UniformBuffers::Shutdown() {
	CR_ASSERT(g_data != nullptr, "UniformBuffers are already shutdown");

	for(int32_t i = 0; i < cegraph::Constants::c_maxFramesInFlight; ++i) {
		vmaDestroyBuffer(GetContext().Allocator, g_data->Buffers[i], g_data->Allocations[i]);
	}

	delete g_data;
}

void cegraph::UniformBuffers::Update() {
	CR_ASSERT(g_data != nullptr, "UniformBuffers are not initialized");

	g_data->CurrentFrame = (g_data->CurrentFrame + 1) % cegraph::Constants::c_maxFramesInFlight;
	g_data->CurrentData  = g_data->Data[g_data->CurrentFrame];
}

cegraph::UniformBuffers::Mapping cegraph::UniformBuffers::Map(uint32_t a_initialSize) {
	CR_ASSERT(g_data != nullptr, "UniformBuffers are not initialized");

	// always start at a 256 byte alignment
	g_data->CurrentData =
	    reinterpret_cast<std::byte*>((reinterpret_cast<uint64_t>(g_data->CurrentData) + 255ull) & (~255ull));

	Mapping result;
	result.Buffer = g_data->Buffers[g_data->CurrentFrame];
	result.Offset = g_data->CurrentData - g_data->Data[g_data->CurrentFrame];
	result.Data   = g_data->CurrentData;

	Append(result, a_initialSize);

	return result;
}

void cegraph::UniformBuffers::Append(Mapping& a_map, uint32_t a_additionalSize) {
	a_map.Size += a_additionalSize;
	g_data->CurrentData += a_additionalSize;
}