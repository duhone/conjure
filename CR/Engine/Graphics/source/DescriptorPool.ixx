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

	CR::Engine::Graphics::Handles::DescriptorSet Create(const VkDescriptorSetLayout& a_layout);
	void Delete(CR::Engine::Graphics::Handles::DescriptorSet a_set);
	void UpdateDescriptorSets(const VkSampler& a_sampler, const std::span<VkImageView> a_imageViews,
	                          const std::span<uint16_t> a_textureIndices);
}    // namespace CR::Engine::Graphics::DescriptorPool

module :private;

using namespace CR::Engine::Core::Literals;

namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	constexpr uint32_t c_maxDescriptorSets = 64;

	VkDescriptorPool m_pool{};
	cecore::BitSet<c_maxDescriptorSets> m_used;
	std::array<VkDescriptorSet, c_maxDescriptorSets> m_descriptorSets;
}    // namespace

void cegraph::DescriptorPool::Initialize() {
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

	VkResult result = vkCreateDescriptorPool(GetContext().Device, &poolInfo, nullptr, &m_pool);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool");
}

void cegraph::DescriptorPool::Shutdown() {
	CR_ASSERT(m_used.empty(), "Not all descriptor sets were released.");

	vkDestroyDescriptorPool(GetContext().Device, m_pool, nullptr);
	m_pool = nullptr;
}

cegraph::Handles::DescriptorSet cegraph::DescriptorPool::Create(const VkDescriptorSetLayout& a_layout) {
	cegraph::Handles::DescriptorSet set{m_used.FindNotInSet()};
	CR_ASSERT(set.isValid(), "ran out of descriptor sets");
	m_used.insert(set.asInt());

	VkDescriptorSetAllocateInfo info{};
	ClearStruct(info);
	info.descriptorPool     = m_pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts        = &a_layout;

	VkResult result = vkAllocateDescriptorSets(GetContext().Device, &info, &m_descriptorSets[set.asInt()]);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set");

	return set;
}

void cegraph::DescriptorPool::Delete(CR::Engine::Graphics::Handles::DescriptorSet a_set) {
	CR_ASSERT(a_set.isValid(), "can't delete an invalid descriptor set");
	CR_ASSERT(m_used.contains(a_set.asInt()), "can't delete an invalid descriptor set");
	m_used.erase(a_set.asInt());
}

void cegraph::DescriptorPool::UpdateDescriptorSets(const VkSampler& a_sampler,
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
		writeSet.dstBinding      = 0;
		writeSet.dstArrayElement = a_textureIndices[i];
		writeSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSet.descriptorCount = 1;
		writeSet.pImageInfo      = &imgInfo;
	}

	for(auto set : m_used) {
		for(auto& writeSet : writeSets) { writeSet.dstSet = m_descriptorSets[set]; }
		vkUpdateDescriptorSets(GetContext().Device, (uint32_t)writeSets.size(), writeSets.data(), 0, nullptr);
	}
}
