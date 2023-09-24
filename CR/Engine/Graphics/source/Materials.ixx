module;

#include "generated/graphics/materials_generated.h"

#include "flatbuffers/idl.h"

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.Materials;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Shaders;
import CR.Engine.Graphics.Utils;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import <vector>;

namespace CR::Engine::Graphics {
	export class Materials {
	  public:
		Materials() = default;
		Materials(const Context& a_context, const Shaders& a_shaders, VkRenderPass a_renderPass);
		~Materials();
		Materials(const Materials&)               = delete;
		Materials(Materials&& a_other)            = delete;
		Materials& operator=(const Materials&)    = delete;
		Materials& operator=(Materials&& a_other) = delete;

	  private:
		const Context& m_context;

		VkPipelineLayout m_pipeLineLayout;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkSampler m_sampler;
		std::vector<VkPipeline> m_pipelines;

		uint32_t m_lastTextureVersion{0};
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace ceplat  = CR::Engine::Platform;
namespace cegraph = CR::Engine::Graphics;

cegraph::Materials::Materials(const Context& a_context, const Shaders& a_shaders, VkRenderPass a_renderPass) :
    m_context(a_context) {
	auto& assetService = cecore::GetService<ceasset::Service>();

	VkSpecializationMapEntry fragSpecInfoEntrys;
	fragSpecInfoEntrys.constantID = 0;
	fragSpecInfoEntrys.offset     = 0;
	fragSpecInfoEntrys.size       = sizeof(int32_t);

	VkSpecializationInfo fragSpecInfo;
	fragSpecInfo.dataSize      = sizeof(c_maxTextures);
	fragSpecInfo.pData         = &c_maxTextures;
	fragSpecInfo.mapEntryCount = 1;
	fragSpecInfo.pMapEntries   = &fragSpecInfoEntrys;

	// TODO: may want to just set the viewport and scissor here for platforms that are
	// fixed window size, instead of using dynamic states.

	VkPipelineViewportStateCreateInfo viewPortInfo;
	ClearStruct(viewPortInfo);
	viewPortInfo.pViewports    = nullptr;
	viewPortInfo.viewportCount = 0;
	viewPortInfo.pScissors     = nullptr;
	viewPortInfo.scissorCount  = 0;

	VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState;
	ClearStruct(dynamicState);
	dynamicState.dynamicStateCount = (uint32_t)std::size(dynamicStates);
	dynamicState.pDynamicStates    = dynamicStates;

	VkPipelineRasterizationStateCreateInfo rasterInfo;
	ClearStruct(rasterInfo);
	rasterInfo.cullMode         = VK_CULL_MODE_NONE;
	rasterInfo.lineWidth        = 1.0f;
	rasterInfo.depthClampEnable = false;
	rasterInfo.polygonMode      = VkPolygonMode::VK_POLYGON_MODE_FILL;

	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	ClearStruct(multisampleInfo);
	multisampleInfo.alphaToCoverageEnable = true;
	multisampleInfo.rasterizationSamples  = VK_SAMPLE_COUNT_4_BIT;
	multisampleInfo.sampleShadingEnable   = true;
	multisampleInfo.minSampleShading      = 1.0f;

	VkPipelineColorBlendAttachmentState blendAttachState;
	blendAttachState.blendEnable    = false;
	blendAttachState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo blendStateInfo;
	ClearStruct(blendStateInfo);
	blendStateInfo.pAttachments    = &blendAttachState;
	blendStateInfo.attachmentCount = 1;

	VkSamplerCreateInfo samplerInfo;
	ClearStruct(samplerInfo);
	samplerInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.minFilter        = VK_FILTER_LINEAR;
	samplerInfo.magFilter        = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.anisotropyEnable = false;

	auto result = vkCreateSampler(a_context.Device, &samplerInfo, nullptr, &m_sampler);
	CR_ASSERT(result == VK_SUCCESS, "failed to create a sampler");

	// Have to pass one sampler per descriptor. but only using one sampler, so just have to duplicate
	std::vector<VkSampler> samplers;
	samplers.reserve(c_maxTextures);
	for(int32_t i = 0; i < c_maxTextures; ++i) { samplers.push_back(m_sampler); }

	VkDescriptorSetLayoutBinding dslBinding[1];
	dslBinding[0].binding            = 0;
	dslBinding[0].descriptorCount    = c_maxTextures;
	dslBinding[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	dslBinding[0].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
	dslBinding[0].pImmutableSamplers = samplers.data();

	VkDescriptorSetLayoutCreateInfo dslInfo;
	ClearStruct(dslInfo);
	dslInfo.bindingCount = (uint32_t)std::size(dslBinding);
	dslInfo.pBindings    = dslBinding;

	result = vkCreateDescriptorSetLayout(m_context.Device, &dslInfo, nullptr, &m_descriptorSetLayout);
	CR_ASSERT(result == VK_SUCCESS, "failed to create a descriptor set layout");

	VkPipelineLayoutCreateInfo layoutInfo;
	ClearStruct(layoutInfo);
	layoutInfo.pushConstantRangeCount = 0;
	layoutInfo.pPushConstantRanges    = nullptr;
	layoutInfo.setLayoutCount         = 1;
	layoutInfo.pSetLayouts            = &m_descriptorSetLayout;

	result = vkCreatePipelineLayout(m_context.Device, &layoutInfo, nullptr, &m_pipeLineLayout);
	CR_ASSERT(result == VK_SUCCESS, "failed to create a pipeline layout");

	VkPipelineInputAssemblyStateCreateInfo vertAssemblyInfo;
	ClearStruct(vertAssemblyInfo);
	vertAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

	assetService.LoadSingle(
	    ceasset::Service::Partitions::Graphics, cecore::C_Hash64("materials.json"),
	    [&](uint64_t a_hash, std::string_view a_path, const std::span<std::byte> a_data) {
		    flatbuffers::Parser parser;
		    ceplat::MemoryMappedFile schemaFile(SCHEMAS_MATERIALS);
		    std::string schemaData((const char*)schemaFile.data(), schemaFile.size());
		    parser.Parse(schemaData.c_str());
		    std::string flatbufferJson((const char*)a_data.data(), a_data.size());
		    parser.ParseJson(flatbufferJson.c_str());
		    CR_ASSERT(parser.BytesConsumed() <= a_data.size(), "buffer overrun loading materials.json");
		    auto materials = Flatbuffers::GetMaterials(parser.builder_.GetBufferPointer());

		    for(const auto& mat : *materials->mats()) {
			    auto vertShader = a_shaders.GetShader(cecore::Hash64(mat->vertex_shader()->c_str()));
			    auto fragShader = a_shaders.GetShader(cecore::Hash64(mat->fragment_shader()->c_str()));

			    VkPipelineShaderStageCreateInfo shaderPipeInfo[2];
			    ClearStruct(shaderPipeInfo[0]);
			    ClearStruct(shaderPipeInfo[1]);
			    shaderPipeInfo[0].module              = vertShader;
			    shaderPipeInfo[0].pName               = "main";
			    shaderPipeInfo[0].stage               = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
			    shaderPipeInfo[0].pSpecializationInfo = nullptr;
			    shaderPipeInfo[1].module              = fragShader;
			    shaderPipeInfo[1].pName               = "main";
			    shaderPipeInfo[1].stage               = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
			    shaderPipeInfo[1].pSpecializationInfo = &fragSpecInfo;

			    VkGraphicsPipelineCreateInfo pipeInfo;
			    ClearStruct(pipeInfo);
			    pipeInfo.layout              = m_pipeLineLayout;
			    pipeInfo.pColorBlendState    = &blendStateInfo;
			    pipeInfo.pInputAssemblyState = &vertAssemblyInfo;
			    pipeInfo.pMultisampleState   = &multisampleInfo;
			    pipeInfo.pRasterizationState = &rasterInfo;
			    // pipeInfo.pVertexInputState   = &vertInputInfo;
			    pipeInfo.pViewportState = &viewPortInfo;
			    pipeInfo.stageCount     = 2;
			    pipeInfo.pStages        = shaderPipeInfo;
			    pipeInfo.renderPass     = a_renderPass;

			    result = vkCreateGraphicsPipelines(m_context.Device, VK_NULL_HANDLE, 1, &pipeInfo, nullptr,
			                                       &m_pipelines.emplace_back());
			    CR_ASSERT(result == VK_SUCCESS, "failed to create a graphics pipeline");
		    }
	    });
}

cegraph::Materials::~Materials() {
	for(auto& pipeline : m_pipelines) { vkDestroyPipeline(m_context.Device, pipeline, nullptr); }
	vkDestroyPipelineLayout(m_context.Device, m_pipeLineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_context.Device, m_descriptorSetLayout, nullptr);
	vkDestroySampler(m_context.Device, m_sampler, nullptr);
}
