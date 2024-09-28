module;

#include "generated/graphics/textures_generated.h"

#include "flatbuffers/idl.h"

#include "core/Log.h"

#include "ankerl/unordered_dense.h"
#include "jxl/decode.h"
#include "jxl/resizable_parallel_runner.h"

#include "Core.h"

export module CR.Engine.Graphics.Textures;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Handles;
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
	void Initialize(Context& a_context);
	void Update();
	void Shutdown();

	Handles::Texture GetHandle(uint64_t hash);
	Handles::TextureSet LoadTextureSet(std::span<uint64_t> hashes);
	void ReleaseTextureSet(Handles::TextureSet set);

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
	constexpr uint32_t c_numJpegXlThreads  = 4;

	using TextureSet = cecore::BitSet<cegraph::Constants::c_maxTextures>;

	struct Data {
		cegraph::Context& gContext;

		// Variables for texture sets
		cecore::BitSet<cegraph::Constants::c_maxTextureSets> TextureSetsUsed;
		std::array<TextureSet, cegraph::Constants::c_maxTextureSets> TextureSets;

		// variables for all textures
		cecore::BitSet<cegraph::Constants::c_maxTextures> Used;
		std::array<uint64_t, cegraph::Constants::c_maxTextures> Hashes;
		std::array<uint64_t, cegraph::Constants::c_maxTextures> AssetHashes;
		std::array<std::string, cegraph::Constants::c_maxTextures> DebugNames;
		std::array<fs::path, cegraph::Constants::c_maxTextures> Paths;
		// Should be the union of all used TextureSets
		TextureSet TexturesLoaded;

		// double buffered. one is being written to by cpu, other is transferring cpu to gpu.
		VkBuffer StagingBuffer[2];
		VmaAllocation StagingMemory[2];
		void* StagingData[2];

		// lookups
		ankerl::unordered_dense::map<uint64_t, cegraph::Handles::Texture> HandleLookup;

		JxlDecoder* Decoder{nullptr};
		void* parRunner{nullptr};
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
		Handles::Texture handle{i};
		g_data->DebugNames[i] = textures[i]->name()->c_str();
		g_data->Paths[i]      = (rootPath / textures[i]->path()->c_str());
		g_data->Hashes[i]     = hash;
		std::string hashPath  = textures[i]->path()->c_str();
		std::ranges::replace(hashPath, '\\', '/');
		g_data->AssetHashes[i] = cecore::Hash64(hashPath);

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

	g_data->Decoder = JxlDecoderCreate(nullptr);
	CR_ASSERT(g_data->Decoder != nullptr, "Failed to initialize jpeg xl decoder");
	g_data->parRunner = JxlResizableParallelRunnerCreate(nullptr);
	CR_ASSERT(g_data->parRunner != nullptr, "Failed to initialize jpeg parallel runner");
	JxlResizableParallelRunnerSetThreads(g_data->parRunner, c_numJpegXlThreads);
	JxlDecoderStatus status =
	    JxlDecoderSetParallelRunner(g_data->Decoder, JxlResizableParallelRunner, g_data->parRunner);
	CR_ASSERT(status == JXL_DEC_SUCCESS, "failed to set jpeg xl parallel runner");
}

void cegraph::Textures::Shutdown() {
	CR_ASSERT(g_data != nullptr, "Textures are already shutdown");

	JxlResizableParallelRunnerDestroy(g_data->parRunner);
	JxlDecoderDestroy(g_data->Decoder);

	vmaDestroyBuffer(g_data->gContext.Allocator, g_data->StagingBuffer[0], g_data->StagingMemory[0]);
	vmaDestroyBuffer(g_data->gContext.Allocator, g_data->StagingBuffer[1], g_data->StagingMemory[1]);
	delete g_data;
}

cegraph::Handles::Texture cegraph::Textures::GetHandle(uint64_t hash) {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");
	auto handleIter = g_data->HandleLookup.find(hash);
	CR_ASSERT(handleIter != g_data->HandleLookup.end(), "Texture could not be found");
	return handleIter->second;
}

cegraph::Handles::TextureSet cegraph::Textures::LoadTextureSet(std::span<uint64_t> hashes) {
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

		uint64_t hash = g_data->AssetHashes[texture];
		auto handle   = assetService.GetHandle(hash);

		assetService.Open(handle);
		auto closeFile = cecore::defer([&] { assetService.Close(handle); });

		auto jxlData = assetService.GetData(handle);
		CR_ASSERT(JxlSignatureCheck((uint8_t*)jxlData.data(), jxlData.size()) == JXL_SIG_CODESTREAM,
		          "jpeg xl file {} invalid", g_data->DebugNames[texture]);

		stagingBuffer = (stagingBuffer + 1) % 2;

		JxlDecoderReset(g_data->Decoder);
		JxlDecoderStatus status = JxlDecoderSetCoalescing(g_data->Decoder, JXL_FALSE);
		CR_ASSERT(status == JXL_DEC_SUCCESS, "Failed to set coalescing on jpeg xl decoder");
		status = JxlDecoderSubscribeEvents(g_data->Decoder, JXL_DEC_BASIC_INFO | JXL_DEC_FRAME);
		CR_ASSERT(status == JXL_DEC_SUCCESS, "Failed to subscribe to events on jpeg xl decoder");
		status = JxlDecoderSetInput(g_data->Decoder, (const uint8_t*)jxlData.data(), jxlData.size());
		CR_ASSERT(status == JXL_DEC_SUCCESS, "Failed to set input on jpeg xl decoder");
		JxlDecoderCloseInput(g_data->Decoder);

		JxlBasicInfo basicInfo;
		uint32_t width{};
		uint32_t height{};

		JxlDecoderStatus loopStatus{};
		do {
			loopStatus = JxlDecoderProcessInput(g_data->Decoder);
			switch(loopStatus) {
				case JXL_DEC_BASIC_INFO: {
					status = JxlDecoderGetBasicInfo(g_data->Decoder, &basicInfo);
					CR_ASSERT(status == JXL_DEC_SUCCESS, "Failed to get basic info on jpeg xl decoder");
					width  = basicInfo.xsize;
					height = basicInfo.ysize;
					CR_ASSERT(basicInfo.bits_per_sample == 8, "we only support 8bpc jpeg xl images");
					CR_ASSERT(basicInfo.num_color_channels == 3, "we only support RGBA jpeg xl images");
					CR_ASSERT(basicInfo.alpha_bits == 8, "we only support RGBA jpeg xl images");
				} break;
				case JXL_DEC_FRAME: {
					JxlFrameHeader frameHeader;
					status = JxlDecoderGetFrameHeader(g_data->Decoder, &frameHeader);
					CR_ASSERT(status == JXL_DEC_SUCCESS, "Failed to get frame header on jpeg xl decoder");
				} break;
				case JXL_DEC_FULL_IMAGE:
					break;
				default:
					break;
			}
		} while(loopStatus != JXL_DEC_SUCCESS);

		JxlDecoderReleaseInput(g_data->Decoder);
	}

	g_data->TexturesLoaded = newCombined;
	return Handles::TextureSet(result);
}

void cegraph::Textures::ReleaseTextureSet(Handles::TextureSet set) {
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
