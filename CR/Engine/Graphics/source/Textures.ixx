module;

#include "generated/graphics/textures_generated.h"

#include "flatbuffers/idl.h"

#include "core/Log.h"

#include "ankerl/unordered_dense.h"

#include "Core.h"

export module CR.Engine.Graphics.Textures;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.GraphicsThread;
import CR.Engine.Graphics.Utils;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import <cstring>;
import <filesystem>;

using namespace CR::Engine::Core::Literals;

export namespace CR::Engine::Graphics::Textures {
	using TextureHandle = CR::Engine::Core::Handle<class TextureHandleTag>;

	void Initialize(Context& a_context);
	void Shutdown();

	TextureHandle GetHandle(uint64_t hash);

}    // namespace CR::Engine::Graphics::Textures

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;
namespace ceplat  = CR::Engine::Platform;

namespace fs = std::filesystem;

namespace {
	// Width	Height		max frames.
	//	4K		  4K			1
	//  2K		  2K			4
	//  1K        1K           16
	//  512       512          64
	//  256       256         256
	//  128       128         1024
	// TODO: when we support packed assets, make this 1/4 size when using packed.
	constexpr uint64_t c_stagingBufferSize = 64_MB;

	struct Data {
		cegraph::Context& gContext;

		// Variables for in use textures
		cecore::BitSet<cegraph::Constants::c_maxTextures> PendingLoad;
		std::array<std::atomic_bool, cegraph::Constants::c_maxTextures> LoadComplete;

		// variables for all textures
		cecore::BitSet<cegraph::Constants::c_maxTextures> Used;
		std::array<std::string, cegraph::Constants::c_maxTextures> DebugNames;
		std::array<fs::path, cegraph::Constants::c_maxTextures> Paths;
		std::array<uint16_t, cegraph::Constants::c_maxTextures> RefCounts;

		VkBuffer StagingBuffer;
		VmaAllocation StagingMemory;
		void* StagingData;

		// lookups
		ankerl::unordered_dense::map<uint64_t, cegraph::Textures::TextureHandle> m_handleLookup;
	};

	Data* g_data = nullptr;
}    // namespace

void cegraph::Textures::Initialize(Context& a_context) {
	CR_ASSERT(g_data == nullptr, "Textures are already initialized");
	g_data = new Data{a_context};

	auto& assetService   = cecore::GetService<ceasset::Service>();
	const auto& rootPath = assetService.GetRootPath();

	flatbuffers::Parser parser =
	    assetService.GetData(cecore::C_Hash64("Graphics/textures.json"), SCHEMAS_TEXTURES);

	auto texturesfb = Flatbuffers::GetTextures(parser.builder_.GetBufferPointer());

	const auto& textures = *texturesfb->textures();
	g_data->Used.insertRange(0, (uint16_t)textures.size());
	for(uint32_t i = 0; i < textures.size(); ++i) {
		TextureHandle handle{i};
		g_data->DebugNames[i] = textures[i]->name()->c_str();
		g_data->Paths[i]      = (rootPath / textures[i]->path()->c_str());

		g_data->m_handleLookup[cecore::Hash64(textures[i]->name()->c_str())] = handle;
	}

	memset(g_data->RefCounts.data(), 0, g_data->RefCounts.size() * sizeof(uint16_t));

	VkBufferCreateInfo stagingCreateInfo{};
	ClearStruct(stagingCreateInfo);
	stagingCreateInfo.size  = c_stagingBufferSize;
	stagingCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	VmaAllocationCreateInfo stagingAllocCreateInfo{};
	stagingAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
	stagingAllocCreateInfo.flags =
	    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo stagingAllocInfo{};
	vmaCreateBuffer(g_data->gContext.Allocator, &stagingCreateInfo, &stagingAllocCreateInfo,
	                &g_data->StagingBuffer, &g_data->StagingMemory, &stagingAllocInfo);
	g_data->StagingData = stagingAllocInfo.pMappedData;
}

void cegraph::Textures::Shutdown() {
	CR_ASSERT(g_data != nullptr, "Textures are already shutdown");
	vmaDestroyBuffer(g_data->gContext.Allocator, g_data->StagingBuffer, g_data->StagingMemory);
	delete g_data;
}

cegraph::Textures::TextureHandle cegraph::Textures::GetHandle(uint64_t hash) {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");
	auto handleIter = g_data->m_handleLookup.find(hash);
	CR_ASSERT(handleIter != g_data->m_handleLookup.end(), "Texture could not be found");
	return handleIter->second;
}