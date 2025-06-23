module;

#include "generated/graphics/computePipelines_generated.h"

#include "flatbuffers/idl.h"

#include "ankerl/unordered_dense.h"
#include <function2/function2.hpp>

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.ComputePipelines;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.GraphicsThread;
import CR.Engine.Graphics.Shaders;
import CR.Engine.Graphics.Utils;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import <vector>;

export namespace CR::Engine::Graphics::ComputePipelines {
	void Initialize();
	void FinishInitialize();
	void Shutdown();

	VkPipeline GetPipeline(uint64_t a_hash);
}    // namespace CR::Engine::Graphics::ComputePipelines

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace ceplat  = CR::Engine::Platform;
namespace cegraph = CR::Engine::Graphics;

namespace {
	VkPipelineLayout m_pipeLineLayout;
	VkDescriptorSetLayout m_descriptorSetLayout;
	std::vector<VkPipeline> m_pipelines;

	std::atomic_flag m_ready;

	std::vector<std::string> m_pipelineNames;
	ankerl::unordered_dense::map<uint64_t, uint32_t> m_pipelineLookup;
}    // namespace

void cegraph::ComputePipelines::Initialize() {
	GraphicsThread::EnqueueTask(
	    []() {
		    auto& assetService = cecore::GetService<ceasset::Service>();

		    // TODO: may want to just set the viewport and scissor here for platforms that are
		    // fixed window size, instead of using dynamic states.

		    VkDescriptorSetLayoutBinding dslBinding[5];
		    ClearStruct(dslBinding);
		    dslBinding[0].binding         = 0;
		    dslBinding[0].descriptorCount = 1;
		    dslBinding[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		    dslBinding[0].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
		    dslBinding[1].binding         = 1;
		    dslBinding[1].descriptorCount = 1;
		    dslBinding[1].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		    dslBinding[1].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
		    dslBinding[2].binding         = 2;
		    dslBinding[2].descriptorCount = 1;
		    dslBinding[2].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		    dslBinding[2].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
		    dslBinding[3].binding         = 3;
		    dslBinding[3].descriptorCount = 1;
		    dslBinding[3].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
		    dslBinding[3].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
		    dslBinding[4].binding         = 4;
		    dslBinding[4].descriptorCount = 1;
		    dslBinding[4].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		    dslBinding[4].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;

		    VkDescriptorSetLayoutCreateInfo dslInfo;
		    ClearStruct(dslInfo);
		    dslInfo.bindingCount = (uint32_t)std::size(dslBinding);
		    dslInfo.pBindings    = dslBinding;

		    auto result =
		        vkCreateDescriptorSetLayout(GetContext().Device, &dslInfo, nullptr, &m_descriptorSetLayout);
		    CR_ASSERT(result == VK_SUCCESS, "failed to create a descriptor set layout");

		    VkPipelineLayoutCreateInfo layoutInfo;
		    ClearStruct(layoutInfo);
		    layoutInfo.pushConstantRangeCount = 0;
		    layoutInfo.pPushConstantRanges    = nullptr;
		    layoutInfo.setLayoutCount         = 1;
		    layoutInfo.pSetLayouts            = &m_descriptorSetLayout;

		    result = vkCreatePipelineLayout(GetContext().Device, &layoutInfo, nullptr, &m_pipeLineLayout);
		    CR_ASSERT(result == VK_SUCCESS, "failed to create a pipeline layout");

		    flatbuffers::Parser parser = assetService.GetData(
		        cecore::C_Hash64("Graphics/computePipelines.json"), SCHEMAS_COMPUTEPIPELINES);

		    auto computePipelines = Flatbuffers::GetComputePipelines(parser.builder_.GetBufferPointer());

		    for(const auto& pipe : *computePipelines->pipelines()) {
			    m_pipelineLookup[cecore::Hash64(pipe->name()->c_str())] = (uint32_t)m_pipelineNames.size();
			    m_pipelineNames.emplace_back(pipe->name()->c_str());

			    auto compShader = Shaders::GetShader(cecore::Hash64(pipe->compute_shader()->c_str()));

			    VkPipelineShaderStageCreateInfo shaderPipeInfo;
			    ClearStruct(shaderPipeInfo);
			    shaderPipeInfo.module              = compShader;
			    shaderPipeInfo.pName               = "main";
			    shaderPipeInfo.stage               = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
			    shaderPipeInfo.pSpecializationInfo = nullptr;

			    VkComputePipelineCreateInfo pipeInfo;
			    ClearStruct(pipeInfo);
			    pipeInfo.layout = m_pipeLineLayout;
			    pipeInfo.stage  = shaderPipeInfo;

			    result = vkCreateComputePipelines(GetContext().Device, VK_NULL_HANDLE, 1, &pipeInfo, nullptr,
			                                      &m_pipelines.emplace_back());
			    CR_ASSERT(result == VK_SUCCESS, "failed to create a graphics pipeline");
		    }
	    },
	    m_ready);
}

void cegraph::ComputePipelines::FinishInitialize() {
	m_ready.wait(false);
}

void cegraph::ComputePipelines::Shutdown() {
	for(auto& pipeline : m_pipelines) { vkDestroyPipeline(GetContext().Device, pipeline, nullptr); }
	vkDestroyPipelineLayout(GetContext().Device, m_pipeLineLayout, nullptr);
	vkDestroyDescriptorSetLayout(GetContext().Device, m_descriptorSetLayout, nullptr);
}

VkPipeline cegraph::ComputePipelines::GetPipeline(uint64_t a_hash) {
	auto pipeIter = m_pipelineLookup.find(a_hash);
	CR_ASSERT(pipeIter != m_pipelineLookup.end(), "Could not find compute pipeline");
	return m_pipelines[pipeIter->second];
}