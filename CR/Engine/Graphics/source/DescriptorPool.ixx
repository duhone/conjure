module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.DescriptorPool;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.InternalHandles;
import CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import <span>;
import <vector>;

export namespace CR::Engine::Graphics::DescriptorPool {
	void Initialize();
	void Shutdown();

	VkDescriptorSet CreateDescriptorSet(const VkDescriptorSetLayout& a_layout);
	void UpdateDescriptorSet(const VkDescriptorSet& a_set, const VkSampler& a_sampler,
	                         const std::span<VkImageView> a_imageViews,
	                         const std::span<uint16_t> a_textureIndices);
}    // namespace CR::Engine::Graphics::DescriptorPool

module :private;

using namespace CR::Engine::Core::Literals;

namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	struct Data {
		VkDescriptorPool Pool{};
	};

	Data* g_data = nullptr;
}    // namespace

void cegraph::DescriptorPool::Initialize() {
	CR_ASSERT(g_data == nullptr, "DescriptorPool is already initialized");

	g_data = new Data{};

	VkDescriptorPoolSize poolSize[2];
	poolSize[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSize[0].descriptorCount = 1;
	poolSize[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize[1].descriptorCount = cegraph::Constants::c_maxTextures;

	VkDescriptorPoolCreateInfo poolInfo{};
	ClearStruct(poolInfo);
	poolInfo.poolSizeCount = (uint32_t)std::size(poolSize);
	poolInfo.pPoolSizes    = std::data(poolSize);
	poolInfo.maxSets       = 1;

	VkResult result = vkCreateDescriptorPool(GetContext().Device, &poolInfo, nullptr, &g_data->Pool);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool");
}

void cegraph::DescriptorPool::Shutdown() {
	CR_ASSERT(g_data != nullptr, "DescriptorPool is already shutdown");

	vkDestroyDescriptorPool(GetContext().Device, g_data->Pool, nullptr);

	delete g_data;
}

VkDescriptorSet cegraph::DescriptorPool::CreateDescriptorSet(const VkDescriptorSetLayout& a_layout) {
	VkDescriptorSet set{};

	VkDescriptorSetAllocateInfo info{};
	ClearStruct(info);
	info.descriptorPool     = g_data->Pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts        = &a_layout;

	VkResult result = vkAllocateDescriptorSets(GetContext().Device, &info, &set);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set");

	return set;
}

void cegraph::DescriptorPool::UpdateDescriptorSet(const VkDescriptorSet& a_set, const VkSampler& a_sampler,
                                                  const std::span<VkImageView> a_imageViews,
                                                  const std::span<uint16_t> a_textureIndices) {
	assert(a_imageViews.size() == a_textureIndices.size());

	std::vector<VkWriteDescriptorSet> writeSets;
	std::vector<VkDescriptorImageInfo> imgInfos;
	writeSets.reserve(a_imageViews.size());
	imgInfos.reserve(a_imageViews.size());

	for(uint32_t i = 0; i < a_imageViews.size(); ++i) {
		VkDescriptorImageInfo& imgInfo = imgInfos.emplace_back();
		imgInfo.imageLayout            = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView              = a_imageViews[i];
		imgInfo.sampler                = a_sampler;

		VkWriteDescriptorSet& writeSet = writeSets.emplace_back();
		ClearStruct(writeSet);
		writeSet.dstSet          = a_set;
		writeSet.dstBinding      = 0;
		writeSet.dstArrayElement = a_textureIndices[i];
		writeSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSet.descriptorCount = 1;
		writeSet.pImageInfo      = &imgInfo;
	}

	vkUpdateDescriptorSets(GetContext().Device, (uint32_t)writeSets.size(), writeSets.data(), 0, nullptr);
}
