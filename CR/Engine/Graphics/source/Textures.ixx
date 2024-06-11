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
	void Shutdown();

	TextureHandle GetHandle(uint64_t hash);
	// check IsReady to see if done.
	TextureSetHandle LoadTextureSetAsync(std::span<uint64_t> hashes);
	void ReleaseTextureSet(TextureSetHandle set);

	bool IsReady(TextureSetHandle handle);
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
		std::mutex LoadingMutex;
		cecore::BitSet<cegraph::Constants::c_maxTextureSets> TextureSetsUsed;
		std::array<TextureSet, cegraph::Constants::c_maxTextureSets> TextureSets;

		cecore::BitSet<cegraph::Constants::c_maxTextureSets> LoadPending;
		cecore::BitSet<cegraph::Constants::c_maxTextureSets> LoadComplete;

		cecore::BitSet<cegraph::Constants::c_maxTextureSets> ReleasePending;
		cecore::BitSet<cegraph::Constants::c_maxTextureSets> ReleaseComplete;

		std::array<bool, cegraph::Constants::c_maxTextureSets> TextureSetsReady;

		// variables for all textures
		cecore::BitSet<cegraph::Constants::c_maxTextures> Used;
		std::array<std::string, cegraph::Constants::c_maxTextures> DebugNames;
		std::array<fs::path, cegraph::Constants::c_maxTextures> Paths;
		// Should be the union of all TextureSets that are Ready
		TextureSet TexturesReady;

		VkBuffer StagingBuffer;
		VmaAllocation StagingMemory;
		void* StagingData;

		// lookups
		ankerl::unordered_dense::map<uint64_t, cegraph::Textures::TextureHandle> m_handleLookup;

		std::jthread LoadThread;
		std::atomic_bool LoadThreadDone{false};
	};

	Data* g_data = nullptr;

	TextureSet GenerateCombined(int32_t filter) {
		TextureSet result;

		for(auto set : g_data->TextureSetsUsed) {
			if(set == filter) { continue; }
			result = result | g_data->TextureSets[set];
		}

		return result;
	}

	void HandleReleasedSets() {
		TextureSet toRelease;

		{
			std::scoped_lock loc(g_data->LoadingMutex);
			for(auto released : g_data->ReleasePending) {
				if(g_data->LoadPending.contains(released)) {
					// if pending load just cancel it, nothing else needs be done
					g_data->LoadPending.erase(released);
				} else {
					toRelease.insert(released);
					// TextureSet newSet    = GenerateCombined(released);
					// TextureSet toRelease = (~newSet) & g_data->TexturesReady;
				}
			}
		}

		{
			std::scoped_lock loc(g_data->LoadingMutex);
			for(uint32_t released : toRelease) {
				g_data->ReleasePending.erase(released);
				g_data->ReleaseComplete.insert(released);
			}
		}
	}

	void LoadThreadMain() {
		CR_ASSERT(g_data != nullptr, "Textures aren't initialized");

		while(!g_data->LoadThreadDone.load(std::memory_order_acquire)) { HandleReleasedSets(); }

		HandleReleasedSets();
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
		TextureHandle handle{i};
		g_data->DebugNames[i] = textures[i]->name()->c_str();
		g_data->Paths[i]      = (rootPath / textures[i]->path()->c_str());

		g_data->m_handleLookup[cecore::Hash64(textures[i]->name()->c_str())] = handle;
	}

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

	g_data->LoadThread = std::jthread(LoadThreadMain);
}

void cegraph::Textures::Shutdown() {
	CR_ASSERT(g_data != nullptr, "Textures are already shutdown");

	g_data->LoadThreadDone.store(true, std::memory_order_release);
	g_data->LoadThread.join();

	vmaDestroyBuffer(g_data->gContext.Allocator, g_data->StagingBuffer, g_data->StagingMemory);
	delete g_data;
}

cegraph::Textures::TextureHandle cegraph::Textures::GetHandle(uint64_t hash) {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");
	auto handleIter = g_data->m_handleLookup.find(hash);
	CR_ASSERT(handleIter != g_data->m_handleLookup.end(), "Texture could not be found");
	return handleIter->second;
}

cegraph::Textures::TextureSetHandle cegraph::Textures::LoadTextureSetAsync(std::span<uint64_t> hashes) {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");

	uint32_t result;
	{
		std::scoped_lock loc(g_data->LoadingMutex);
		result = g_data->TextureSetsUsed.FindNotInSet();
		CR_ASSERT(g_data->TextureSets[result].empty(), "New textures set should start empty");

		for(uint64_t hash : hashes) {
			auto handleIter = g_data->m_handleLookup.find(hash);
			CR_ASSERT(handleIter != g_data->m_handleLookup.end(), "Texture could not be found");
			g_data->TextureSets[result].insert(handleIter->second.asInt());
		}

		g_data->LoadPending.insert(result);
	}

	return TextureSetHandle(result);
}

void cegraph::Textures::ReleaseTextureSet(TextureSetHandle set) {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");
	CR_ASSERT(set.isValid(), "invalid texture set handle");
	CR_ASSERT(set.asInt() != g_data->TextureSetsUsed.size(), "Ran out of available texture sets");

	{
		std::scoped_lock loc(g_data->LoadingMutex);
		g_data->ReleasePending.insert(set.asInt());
	}
}

bool cegraph::Textures::IsReady(TextureSetHandle handle) {
	CR_ASSERT_AUDIT(handle.isValid(), "texture set handle not valid");
	CR_ASSERT(handle.asInt() < Constants::c_maxTextureSets, "texture set handle not valid");
	return g_data->TextureSetsReady[handle.asInt()];
}