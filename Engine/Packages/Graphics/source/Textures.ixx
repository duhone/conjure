module;

#include "generated/graphics/textures_generated.h"

#include "flatbuffers/idl.h"

#include "core/Core.h"

#include "ankerl/unordered_dense.h"
#include "webp/decode.h"
#include "webp/demux.h"

#include "Core.h"

export module CR.Engine.Graphics.Textures;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Commands;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Handles;
import CR.Engine.Graphics.GraphicsThread;
import CR.Engine.Graphics.Utils;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import std;
import std.compat;

using namespace CR::Engine::Core::Literals;

export namespace CR::Engine::Graphics::Textures {
	void Initialize();
	// should be first thing in command buffer. for sure before the renderpass begins
	void Update(VkCommandBuffer a_cmdBuffer);
	void Shutdown();

	Handles::Texture GetHandle(uint64_t hash);
	Handles::TextureSet LoadTextureSet(std::span<uint64_t> hashes);
	void ReleaseTextureSet(Handles::TextureSet set);

	// can only get this for loaded textures
	uint32_t GetNumFrames(Handles::Texture a_texture);
	glm::uvec2 GetDimensions(Handles::Texture a_texture);
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

	// Variables for texture sets
	cecore::BitSet<cegraph::Constants::c_maxTextureSets> m_textureSetsUsed;
	std::array<TextureSet, cegraph::Constants::c_maxTextureSets> m_textureSets;

	// variables for all textures
	cecore::BitSet<cegraph::Constants::c_maxTextures> m_used;
	std::array<uint64_t, cegraph::Constants::c_maxTextures> m_hashes;
	std::array<uint64_t, cegraph::Constants::c_maxTextures> m_assetHashes;
	std::array<std::string, cegraph::Constants::c_maxTextures> m_debugNames;
	std::array<VkImage, cegraph::Constants::c_maxTextures> m_images;
	std::array<VmaAllocation, cegraph::Constants::c_maxTextures> m_allocations;
	std::array<VkImageView, cegraph::Constants::c_maxTextures> m_views;
	std::array<uint16_t, cegraph::Constants::c_maxTextures> m_numFrames;
	std::array<glm::uvec2, cegraph::Constants::c_maxTextures> m_dimensions;
	cecore::BitSet<cegraph::Constants::c_maxTextures> m_needsTransferBarrier;

	// Should be the union of all used TextureSets
	TextureSet m_texturesLoaded;

	// double buffered. one is being written to by cpu, other is transferring cpu to gpu.
	VkBuffer m_stagingBuffer[2];
	VmaAllocation m_stagingMemory[2];
	void* m_stagingData[] = {nullptr, nullptr};

	// lookups
	ankerl::unordered_dense::map<uint64_t, cegraph::Handles::Texture> m_handleLookup;

	VkSampler m_sampler{};

	TextureSet GenerateCombined() {
		TextureSet result;

		for(auto set : m_textureSetsUsed) { result = result | m_textureSets[set]; }

		return result;
	}

}    // namespace

void cegraph::Textures::Initialize() {
	CR_ASSERT(m_stagingData[0] == nullptr, "Textures are already initialized");

	flatbuffers::Parser parser =
	    ceasset::GetData(cecore::C_Hash64("Graphics/textures.json"), SCHEMAS_TEXTURES);

	auto texturesfb = Flatbuffers::GetTextures(parser.builder_.GetBufferPointer());

	const auto& textures = *texturesfb->textures();
	m_used.insertRange(0, (uint16_t)textures.size());
	for(uint32_t i = 0; i < textures.size(); ++i) {
		uint64_t hash = cecore::Hash64(textures[i]->name()->c_str());
		Handles::Texture handle{i};
		m_debugNames[i]      = textures[i]->name()->c_str();
		m_hashes[i]          = hash;
		std::string hashPath = textures[i]->path()->c_str();
		std::ranges::replace(hashPath, '\\', '/');
		m_assetHashes[i] = cecore::Hash64(hashPath);

		m_handleLookup[hash] = handle;
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
		vmaCreateBuffer(GetContext().Allocator, &stagingCreateInfo, &stagingAllocCreateInfo,
		                &m_stagingBuffer[i], &m_stagingMemory[i], &stagingAllocInfo);
		m_stagingData[i] = stagingAllocInfo.pMappedData;
	}

	// we only have 1 sampler for now. basic trilinear
	VkSamplerCreateInfo samplerInfo;
	ClearStruct(samplerInfo);
	samplerInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.minFilter        = VK_FILTER_LINEAR;
	samplerInfo.magFilter        = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.anisotropyEnable = false;

	auto result = vkCreateSampler(GetContext().Device, &samplerInfo, nullptr, &m_sampler);
	CR_ASSERT(result == VK_SUCCESS, "failed to create a sampler");
}

void cegraph::Textures::Shutdown() {
	CR_ASSERT(m_stagingData[0] != nullptr, "Textures are already shutdown");

	vkDestroySampler(GetContext().Device, m_sampler, nullptr);

	vmaDestroyBuffer(GetContext().Allocator, m_stagingBuffer[0], m_stagingMemory[0]);
	vmaDestroyBuffer(GetContext().Allocator, m_stagingBuffer[1], m_stagingMemory[1]);
}

cegraph::Handles::Texture cegraph::Textures::GetHandle(uint64_t hash) {
	CR_ASSERT(m_stagingData[0] != nullptr, "Textures not initialized");
	auto handleIter = m_handleLookup.find(hash);
	CR_ASSERT(handleIter != m_handleLookup.end(), "Texture could not be found");
	return handleIter->second;
}

cegraph::Handles::TextureSet cegraph::Textures::LoadTextureSet(std::span<uint64_t> hashes) {
	CR_ASSERT(m_stagingData[0] != nullptr, "Textures not initialized");

	uint32_t result;
	result = m_textureSetsUsed.FindNotInSet();
	CR_ASSERT(m_textureSets[result].empty(), "New textures set should start empty");
	m_textureSetsUsed.insert((uint16_t)result);

	for(uint64_t hash : hashes) {
		auto handleIter = m_handleLookup.find(hash);
		CR_ASSERT(handleIter != m_handleLookup.end(), "Texture could not be found");
		m_textureSets[result].insert(handleIter->second);
	}

	TextureSet newCombined = GenerateCombined();
	TextureSet toLoad      = newCombined ^ m_texturesLoaded;

	uint32_t stagingBuffer{};
	std::atomic_flag loadComplete[2];
	// don't block on first wait call, as no previous task first time;
	loadComplete[0].test_and_set();
	loadComplete[1].test_and_set();

	bool dedicatedTransfer = GetContext().TransferQueueIndex != GetContext().GraphicsQueueIndex;

	for(uint16_t texture : toLoad) {
		CR_ASSERT(m_used.contains(texture), "Tried to load a texture that doesn't exist");

		uint64_t hash = m_assetHashes[texture];
		auto handle   = ceasset::GetHandle(hash);

		ceasset::Open(handle);
		defer({ ceasset::Close(handle); });

		auto webpRawData = ceasset::GetData(handle);

		WebPAnimDecoderOptions decoderOptions;
		WebPAnimDecoderOptionsInit(&decoderOptions);
		decoderOptions.use_threads = true;
		// we want premultiplied alpha, let libwebp take care of, thats what the lowercase rgb mean.
		decoderOptions.color_mode = MODE_rgbA;

		WebPData webpFileData;
		WebPDataInit(&webpFileData);
		webpFileData.bytes = (const uint8_t*)webpRawData.data();
		webpFileData.size  = webpRawData.size();

		WebPAnimDecoder* decoder = WebPAnimDecoderNew(&webpFileData, &decoderOptions);
		CR_ASSERT(decoder != nullptr, "webp file {} invalid", m_debugNames[texture]);
		defer({ WebPAnimDecoderDelete(decoder); });

		WebPAnimInfo animInfo;
		auto webpResult = WebPAnimDecoderGetInfo(decoder, &animInfo);
		CR_ASSERT(webpResult != 0, "webp file {} invalid", m_debugNames[texture]);

		uint32_t width  = animInfo.canvas_width;
		uint32_t height = animInfo.canvas_height;

		stagingBuffer = (stagingBuffer + 1) % 2;

		std::vector<VkBufferImageCopy> frameCopies;
		VkDeviceSize bufferOffset{};
		uint32_t nextLayer{};

		loadComplete[stagingBuffer].wait(false);
		loadComplete[stagingBuffer].clear();

		uint8_t* outputBuffer = (uint8_t*)m_stagingData[stagingBuffer];

		while(WebPAnimDecoderHasMoreFrames(decoder)) {
			int timestamp{};
			uint8_t* decodeBuffer{};
			webpResult = WebPAnimDecoderGetNext(decoder, &decodeBuffer, &timestamp);
			CR_ASSERT(webpResult != 0, "webp file {} invalid, failed to decode a frame",
			          m_debugNames[texture]);

			size_t frameSize = (size_t)width * height * 4;
			CR_ASSERT((bufferOffset + frameSize) < c_stagingBufferSize,
			          "Staging buffer too small to hold webp texture");

			// TODO: we aren't zero copy. webp decodes into its own buffer, we then copy to our staging
			// buffer. It would be nice to decode directly into the staging buffer. Lower level api's kind of
			// allow that, but some issues there too.
			memcpy(outputBuffer + bufferOffset, decodeBuffer, frameSize);

			VkBufferImageCopy& copyInfo              = frameCopies.emplace_back();
			copyInfo.bufferOffset                    = bufferOffset;
			copyInfo.bufferRowLength                 = 0;
			copyInfo.bufferImageHeight               = 0;
			copyInfo.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			copyInfo.imageSubresource.mipLevel       = 0;
			copyInfo.imageSubresource.baseArrayLayer = nextLayer;
			copyInfo.imageSubresource.layerCount     = 1;
			copyInfo.imageOffset                     = {0, 0, 0};
			copyInfo.imageExtent                     = {width, height, 1};

			bufferOffset += frameSize;
			++nextLayer;
		}

		m_numFrames[texture]  = (uint16_t)frameCopies.size();
		m_dimensions[texture] = glm::uvec2{width, height};

		VkImageCreateInfo createInfo;
		ClearStruct(createInfo);
		createInfo.extent.width  = width;
		createInfo.extent.height = height;
		createInfo.extent.depth  = 1;
		createInfo.arrayLayers   = m_numFrames[texture];
		createInfo.mipLevels     = 1;
		createInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
		createInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		createInfo.imageType     = VK_IMAGE_TYPE_2D;
		createInfo.flags         = 0;
		createInfo.format        = VK_FORMAT_R8G8B8A8_SRGB;
		createInfo.samples       = VK_SAMPLE_COUNT_1_BIT;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

		VkImage image{};
		VmaAllocation imageAlloc{};
		VkImageView imageView{};
		VkResult vkResult = vmaCreateImage(GetContext().Allocator, &createInfo, &allocCreateInfo, &image,
		                                   &imageAlloc, nullptr);
		CR_ASSERT(vkResult == VK_SUCCESS, "Failed to create vulkan image");

		VkImageViewCreateInfo viewInfo;
		ClearStruct(viewInfo);
		viewInfo.image                           = image;
		viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		viewInfo.format                          = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel   = 0;
		viewInfo.subresourceRange.levelCount     = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount     = m_numFrames[texture];

		vkResult = vkCreateImageView(GetContext().Device, &viewInfo, nullptr, &imageView);
		CR_ASSERT(vkResult == VK_SUCCESS, "Failed to create vulkan image view");

		m_images[texture]      = image;
		m_allocations[texture] = imageAlloc;
		m_views[texture]       = imageView;

		// clearing again, mostly just want the memory barrier. be sure task see latest on all our variables.
		loadComplete[stagingBuffer].clear();
		GraphicsThread::EnqueueTask(
		    [dedicatedTransfer = dedicatedTransfer, texture = texture, image = image,
		     buffer      = m_stagingBuffer[stagingBuffer],
		     frameCopies = std::move(frameCopies)](VkCommandBuffer& cmdBuffer) mutable {
			    Commands::TransitionToDst(cmdBuffer, image, m_numFrames[texture]);
			    Commands::CopyBufferToImg(cmdBuffer, buffer, image, frameCopies);
			    if(dedicatedTransfer) {
				    Commands::TransitionToGraphicsQueue(cmdBuffer, image, m_numFrames[texture]);
			    } else {
				    Commands::TransitionToReadOptimal(cmdBuffer, image, m_numFrames[texture]);
			    }
		    },
		    loadComplete[stagingBuffer]);

		if(dedicatedTransfer) { m_needsTransferBarrier.insert(texture); }

		stagingBuffer = (stagingBuffer + 1) % 2;
	}

	// wait for last of remaining loads to finish.
	loadComplete[0].wait(false);
	loadComplete[1].wait(false);

	m_texturesLoaded = newCombined;
	return Handles::TextureSet(result);
}

void cegraph::Textures::ReleaseTextureSet(Handles::TextureSet set) {
	CR_ASSERT(m_stagingData[0] != nullptr, "Textures not initialized");
	CR_ASSERT(set.isValid(), "invalid texture set handle");
	CR_ASSERT(set != m_textureSetsUsed.size(), "Ran out of available texture sets");

	vkDeviceWaitIdle(GetContext().Device);

	m_textureSets[set].clear();
	m_textureSetsUsed.erase(set);

	TextureSet newLoaded = GenerateCombined();
	TextureSet toUnLoad  = newLoaded ^ m_texturesLoaded;

	for(uint16_t texture : toUnLoad) {
		m_needsTransferBarrier.erase(texture);

		vkDestroyImageView(GetContext().Device, m_views[texture], nullptr);
		vmaDestroyImage(GetContext().Allocator, m_images[texture], m_allocations[texture]);
		m_views[texture]       = VK_NULL_HANDLE;
		m_images[texture]      = VK_NULL_HANDLE;
		m_allocations[texture] = VK_NULL_HANDLE;
	}

	m_texturesLoaded = newLoaded;
}

void cegraph::Textures::Update(VkCommandBuffer a_cmdBuffer) {
	CR_ASSERT(m_stagingData[0] != nullptr, "Textures not initialized");

	for(uint16_t texture : m_needsTransferBarrier) {
		Commands::TransitionFromTransferQueue(a_cmdBuffer, m_images[texture], m_numFrames[texture]);
	}
	m_needsTransferBarrier.clear();

	VkWriteDescriptorSet writeSet;
	std::vector<VkDescriptorImageInfo> imgInfos;
	imgInfos.reserve(m_views.size());

	for(uint32_t i = 0; i < m_views.size(); ++i) {
		VkDescriptorImageInfo& imgInfo = imgInfos.emplace_back();
		imgInfo.imageLayout            = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView              = m_views[i];
		imgInfo.sampler                = m_sampler;
	}

	ClearStruct(writeSet);
	writeSet.dstBinding      = 0;
	writeSet.dstArrayElement = 0;
	writeSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSet.descriptorCount = (uint32_t)imgInfos.size();
	writeSet.pImageInfo      = imgInfos.data();
	writeSet.dstSet          = GetContext().GlobalDescriptorSet;
	vkUpdateDescriptorSets(GetContext().Device, 1, &writeSet, 0, nullptr);
}

uint32_t cegraph::Textures::GetNumFrames(Handles::Texture a_texture) {
	CR_ASSERT(m_stagingData[0] != nullptr, "Textures not initialized");
	CR_ASSERT(m_texturesLoaded.contains(a_texture), "Texture not loaded, cant get number of frames");

	return m_numFrames[a_texture];
}

glm::uvec2 cegraph::Textures::GetDimensions(Handles::Texture a_texture) {
	CR_ASSERT(m_stagingData[0] != nullptr, "Textures not initialized");
	CR_ASSERT(m_texturesLoaded.contains(a_texture), "Texture not loaded, cant get number of frames");

	return m_dimensions[a_texture];
}
