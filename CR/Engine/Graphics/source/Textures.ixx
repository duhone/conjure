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

import <atomic>;
import <cstring>;
import <filesystem>;
import <span>;
import <thread>;

using namespace CR::Engine::Core::Literals;

export namespace CR::Engine::Graphics::Textures {
	using TextureHandle    = CR::Engine::Core::Handle<class TextureHandleTag>;
	using TextureSetHandle = CR::Engine::Core::Handle<class TextureSetHandleTag>;

	void Initialize(Context& a_context);
	void Update();
	void Shutdown();

	TextureHandle GetHandle(uint64_t hash);
	TextureSetHandle LoadTextureSet(std::span<uint64_t> hashes);
	void ReleaseTextureSet(TextureSetHandle set);

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

	using TextureSet = cecore::BitSet<cegraph::Constants::c_maxTextures>;

	struct Data {
		cegraph::Context& gContext;

		// Variables for texture sets
		cecore::BitSet<cegraph::Constants::c_maxTextureSets> TextureSetsUsed;
		std::array<TextureSet, cegraph::Constants::c_maxTextureSets> TextureSets;

		// variables for all textures
		cecore::BitSet<cegraph::Constants::c_maxTextures> Used;
		std::array<uint64_t, cegraph::Constants::c_maxTextures> Hashes;
		std::array<std::string, cegraph::Constants::c_maxTextures> DebugNames;
		std::array<fs::path, cegraph::Constants::c_maxTextures> Paths;
		// Should be the union of all used TextureSets
		TextureSet TexturesLoaded;

		// double buffered. one is being written to by cpu, other is transferring cpu to gpu.
		VkBuffer StagingBuffer[2];
		VmaAllocation StagingMemory[2];
		void* StagingData[2];

		// lookups
		ankerl::unordered_dense::map<uint64_t, cegraph::Textures::TextureHandle> HandleLookup;
	};

	Data* g_data = nullptr;

	TextureSet GenerateCombined() {
		TextureSet result;

		for(auto set : g_data->TextureSetsUsed) { result = result | g_data->TextureSets[set]; }

		return result;
	}

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
		uint64_t hash = cecore::Hash64(textures[i]->name()->c_str());
		TextureHandle handle{i};
		g_data->DebugNames[i] = textures[i]->name()->c_str();
		g_data->Paths[i]      = (rootPath / textures[i]->path()->c_str());
		g_data->Hashes[i]     = hash;

		g_data->HandleLookup[hash] = handle;
	}

	for(int32_t i = 0; i < 2; ++i) {
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
		                &g_data->StagingBuffer[i], &g_data->StagingMemory[i], &stagingAllocInfo);
		g_data->StagingData[i] = stagingAllocInfo.pMappedData;
	}
}

void cegraph::Textures::Shutdown() {
	CR_ASSERT(g_data != nullptr, "Textures are already shutdown");

	vmaDestroyBuffer(g_data->gContext.Allocator, g_data->StagingBuffer[0], g_data->StagingMemory[0]);
	vmaDestroyBuffer(g_data->gContext.Allocator, g_data->StagingBuffer[1], g_data->StagingMemory[1]);
	delete g_data;
}

cegraph::Textures::TextureHandle cegraph::Textures::GetHandle(uint64_t hash) {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");
	auto handleIter = g_data->HandleLookup.find(hash);
	CR_ASSERT(handleIter != g_data->HandleLookup.end(), "Texture could not be found");
	return handleIter->second;
}

cegraph::Textures::TextureSetHandle cegraph::Textures::LoadTextureSet(std::span<uint64_t> hashes) {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");

	uint32_t result;
	result = g_data->TextureSetsUsed.FindNotInSet();
	CR_ASSERT(g_data->TextureSets[result].empty(), "New textures set should start empty");
	g_data->TextureSetsUsed.insert((uint16_t)result);

	auto& assetService = cecore::GetService<ceasset::Service>();
	for(uint64_t hash : hashes) {
		auto handleIter = g_data->HandleLookup.find(hash);
		CR_ASSERT(handleIter != g_data->HandleLookup.end(), "Texture could not be found");
		g_data->TextureSets[result].insert(handleIter->second.asInt());
	}

	TextureSet newCombined = GenerateCombined();
	TextureSet toLoad      = newCombined ^ g_data->TexturesLoaded;

	uint32_t stagingBuffer{};
	for(uint16_t texture : toLoad) {
		CR_ASSERT(g_data->Used.contains(texture), "Tried to load a texture that doesn't exist");

		uint64_t hash = g_data->Hashes[texture];
		auto handle   = assetService.GetHandle(hash);

		assetService.Open(handle);
		cecore::defer([&] { assetService.Close(handle); });

		auto jxlData = assetService.GetData(handle);

		stagingBuffer = (stagingBuffer + 1) % 2;
	}

	g_data->TexturesLoaded = newCombined;
	return TextureSetHandle(result);
}

void cegraph::Textures::ReleaseTextureSet(TextureSetHandle set) {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");
	CR_ASSERT(set.isValid(), "invalid texture set handle");
	CR_ASSERT(set.asInt() != g_data->TextureSetsUsed.size(), "Ran out of available texture sets");

	g_data->TextureSets[set.asInt()].clear();
	g_data->TextureSetsUsed.erase(set.asInt());

	TextureSet newLoaded = GenerateCombined();
	TextureSet toUnLoad  = newLoaded ^ g_data->TexturesLoaded;

	g_data->TexturesLoaded = newLoaded;
}

void cegraph::Textures::Update() {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");
}
