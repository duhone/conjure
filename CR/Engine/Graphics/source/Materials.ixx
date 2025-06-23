module;

#include "generated/graphics/materials_generated.h"

#include "flatbuffers/idl.h"

#include "ankerl/unordered_dense.h"
#include <function2/function2.hpp>

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.Materials;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.GraphicsThread;
import CR.Engine.Graphics.InternalHandles;
import CR.Engine.Graphics.Shaders;
import CR.Engine.Graphics.Utils;
import CR.Engine.Graphics.Textures;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import <vector>;

export namespace CR::Engine::Graphics::Materials {
	void Initialize(VkRenderPass a_renderPass);
	void FinishInitialize();
	void Shutdown();

	Handles::Material GetMaterial(std::string_view a_name);

	void Bind(Handles::Material a_material, VkCommandBuffer& a_cmdBuffer);
}    // namespace CR::Engine::Graphics::Materials

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace ceplat  = CR::Engine::Platform;
namespace cegraph = CR::Engine::Graphics;

namespace {
	VkPipelineLayout m_pipeLineLayout;
	std::vector<VkPipeline> m_pipelines;
	ankerl::unordered_dense::map<std::string, uint16_t> m_materialLookup;

	std::atomic_flag m_ready;

	VkFormat toVkFormat(cegraph::Flatbuffers::VertAttrFormat a_arg) {
		switch(a_arg) {
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::f32x1:
				return VK_FORMAT_R32_SFLOAT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::f32x2:
				return VK_FORMAT_R32G32_SFLOAT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::f32x3:
				return VK_FORMAT_R32G32B32_SFLOAT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::f32x4:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::unorm16x1:
				return VK_FORMAT_R16_UNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::unorm16x2:
				return VK_FORMAT_R16G16_UNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::unorm16x3:
				return VK_FORMAT_R16G16B16_UNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::unorm16x4:
				return VK_FORMAT_R16G16B16A16_UNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::unorm8x1:
				return VK_FORMAT_R8_UNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::unorm8x2:
				return VK_FORMAT_R8G8_UNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::unorm8x3:
				return VK_FORMAT_R8G8B8_UNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::unorm8x4:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::snorm16x1:
				return VK_FORMAT_R16_SNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::snorm16x2:
				return VK_FORMAT_R16G16_SNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::snorm16x3:
				return VK_FORMAT_R16G16B16_SNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::snorm16x4:
				return VK_FORMAT_R16G16B16A16_SNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::snorm8x1:
				return VK_FORMAT_R8_SNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::snorm8x2:
				return VK_FORMAT_R8G8_SNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::snorm8x3:
				return VK_FORMAT_R8G8B8_SNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::snorm8x4:
				return VK_FORMAT_R8G8B8A8_SNORM;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::uint16x1:
				return VK_FORMAT_R16_UINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::uint16x2:
				return VK_FORMAT_R16G16_UINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::uint16x3:
				return VK_FORMAT_R16G16B16_UINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::uint16x4:
				return VK_FORMAT_R16G16B16A16_UINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::uint8x1:
				return VK_FORMAT_R8_UINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::uint8x2:
				return VK_FORMAT_R8G8_UINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::uint8x3:
				return VK_FORMAT_R8G8B8_UINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::uint8x4:
				return VK_FORMAT_R8G8B8A8_UINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::sint16x1:
				return VK_FORMAT_R16_SINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::sint16x2:
				return VK_FORMAT_R16G16_SINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::sint16x3:
				return VK_FORMAT_R16G16B16_SINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::sint16x4:
				return VK_FORMAT_R16G16B16A16_SINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::sint8x1:
				return VK_FORMAT_R8_SINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::sint8x2:
				return VK_FORMAT_R8G8_SINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::sint8x3:
				return VK_FORMAT_R8G8B8_SINT;
			case CR::Engine::Graphics::Flatbuffers::VertAttrFormat::sint8x4:
				return VK_FORMAT_R8G8B8A8_SINT;
			default:
				CR_ERROR("Unknown Vertex attrib format: {}", (int)a_arg);
				return VK_FORMAT_UNDEFINED;
				break;
		}
	}
}    // namespace

void cegraph::Materials::Initialize(VkRenderPass a_renderPass) {
	GraphicsThread::EnqueueTask(
	    [a_renderPass]() {
		    auto& assetService = cecore::GetService<ceasset::Service>();

		    VkSpecializationMapEntry fragSpecInfoEntrys;
		    fragSpecInfoEntrys.constantID = 0;
		    fragSpecInfoEntrys.offset     = 0;
		    fragSpecInfoEntrys.size       = sizeof(int32_t);

		    VkSpecializationInfo fragSpecInfo;
		    fragSpecInfo.dataSize      = sizeof(Constants::c_maxTextures);
		    fragSpecInfo.pData         = &Constants::c_maxTextures;
		    fragSpecInfo.mapEntryCount = 1;
		    fragSpecInfo.pMapEntries   = &fragSpecInfoEntrys;

		    // TODO: may want to just set the viewport and scissor here for platforms that are
		    // fixed window size, instead of using dynamic states.

		    VkPipelineViewportStateCreateInfo viewPortInfo;
		    ClearStruct(viewPortInfo);
		    viewPortInfo.pViewports    = nullptr;
		    viewPortInfo.viewportCount = 1;
		    viewPortInfo.pScissors     = nullptr;
		    viewPortInfo.scissorCount  = 1;

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
		    ClearStruct(blendAttachState);
		    blendAttachState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		    blendAttachState.blendEnable         = false;
		    blendAttachState.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		    VkPipelineColorBlendStateCreateInfo blendStateInfo;
		    ClearStruct(blendStateInfo);
		    blendStateInfo.pAttachments    = &blendAttachState;
		    blendStateInfo.attachmentCount = 1;

		    VkPipelineLayoutCreateInfo layoutInfo;
		    ClearStruct(layoutInfo);
		    layoutInfo.pushConstantRangeCount = 0;
		    layoutInfo.pPushConstantRanges    = nullptr;
		    layoutInfo.setLayoutCount         = 1;
		    layoutInfo.pSetLayouts            = &cegraph::Textures::GetDescriptorSetLayout();

		    VkResult result =
		        vkCreatePipelineLayout(GetContext().Device, &layoutInfo, nullptr, &m_pipeLineLayout);
		    CR_ASSERT(result == VK_SUCCESS, "failed to create a pipeline layout");

		    VkPipelineInputAssemblyStateCreateInfo vertAssemblyInfo;
		    ClearStruct(vertAssemblyInfo);
		    vertAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

		    flatbuffers::Parser parser =
		        assetService.GetData(cecore::C_Hash64("Graphics/materials.json"), SCHEMAS_MATERIALS);
		    auto materials = Flatbuffers::GetMaterials(parser.builder_.GetBufferPointer());

		    for(const auto& mat : *materials->mats()) {
			    auto vertShader = Shaders::GetShader(cecore::Hash64(mat->vertex_shader()->c_str()));
			    auto fragShader = Shaders::GetShader(cecore::Hash64(mat->fragment_shader()->c_str()));

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

			    VkPipelineVertexInputStateCreateInfo vertInputInfo;
			    ClearStruct(vertInputInfo);
			    std::vector<VkVertexInputBindingDescription> vertBindings;
			    vertBindings.reserve(mat->bindings()->size());
			    for(const auto& binding : *mat->bindings()) {
				    auto& newBinding   = vertBindings.emplace_back();
				    newBinding.binding = binding->binding();
				    newBinding.stride  = binding->stride();
				    switch(binding->rate()) {
					    case Flatbuffers::VertInputRate::PerVertex:
						    newBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
						    break;
					    case Flatbuffers::VertInputRate::PerInstance:
						    newBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
						    break;
					    default:
						    break;
				    }
			    }
			    vertInputInfo.vertexBindingDescriptionCount = (uint32_t)vertBindings.size();
			    vertInputInfo.pVertexBindingDescriptions    = vertBindings.data();

			    std::vector<VkVertexInputAttributeDescription> vertAttribs;
			    vertAttribs.reserve(mat->attr_descs()->size());
			    for(const auto& desc : *mat->attr_descs()) {
				    auto& newDesc    = vertAttribs.emplace_back();
				    newDesc.location = desc->location();
				    newDesc.binding  = desc->binding();
				    newDesc.offset   = desc->offset();
				    newDesc.format   = toVkFormat(desc->format());
			    }
			    vertInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertAttribs.size();
			    vertInputInfo.pVertexAttributeDescriptions    = vertAttribs.data();

			    VkGraphicsPipelineCreateInfo pipeInfo;
			    ClearStruct(pipeInfo);
			    pipeInfo.layout              = m_pipeLineLayout;
			    pipeInfo.pColorBlendState    = &blendStateInfo;
			    pipeInfo.pInputAssemblyState = &vertAssemblyInfo;
			    pipeInfo.pMultisampleState   = &multisampleInfo;
			    pipeInfo.pRasterizationState = &rasterInfo;
			    pipeInfo.pDynamicState       = &dynamicState;
			    pipeInfo.pVertexInputState   = &vertInputInfo;
			    pipeInfo.pViewportState      = &viewPortInfo;
			    pipeInfo.stageCount          = 2;
			    pipeInfo.pStages             = shaderPipeInfo;
			    pipeInfo.renderPass          = a_renderPass;

			    result = vkCreateGraphicsPipelines(GetContext().Device, VK_NULL_HANDLE, 1, &pipeInfo, nullptr,
			                                       &m_pipelines.emplace_back());
			    CR_ASSERT(result == VK_SUCCESS, "failed to create a graphics pipeline");

			    m_materialLookup.emplace(mat->name()->string_view(), uint16_t(m_pipelines.size() - 1));
		    }
	    },
	    m_ready);
}
void cegraph::Materials::FinishInitialize() {
	m_ready.wait(false);
}

void cegraph::Materials::Shutdown() {
	for(auto& pipeline : m_pipelines) { vkDestroyPipeline(GetContext().Device, pipeline, nullptr); }
	vkDestroyPipelineLayout(GetContext().Device, m_pipeLineLayout, nullptr);
}

cegraph::Handles::Material cegraph::Materials::GetMaterial(std::string_view a_name) {
	auto matIter = m_materialLookup.find(std::string(a_name));
	CR_ASSERT(matIter != m_materialLookup.end(), "asked for material that didn't existd. {}", a_name);
	return Handles::Material{matIter->second};
}

void cegraph::Materials::Bind(Handles::Material a_material, VkCommandBuffer& a_cmdBuffer) {
	CR_ASSERT(a_material.isValid() && a_material.asInt() < m_pipelines.size(),
	          "Tried to bind an invalid material");

	vkCmdBindPipeline(a_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[a_material.asInt()]);

	vkCmdBindDescriptorSets(a_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeLineLayout, 0, 1,
	                        &cegraph::Textures::GetDescriptorSet(), 0, nullptr);

	/* won't need descriptor for textures anymore. redo this code if we need buffers in future
	VkWriteDescriptorSet writeSet{};
	std::array<VkDescriptorImageInfo, Constants::c_maxTextures> imgInfos{};

	std::span<VkImageView> imgViews = cegraph::Textures::GetImageViews();
	CR_ASSERT(imgViews.size() == Constants::c_maxTextures, "Unexpected number of texture image views");

	VkSampler sampler = cegraph::Textures::GetSampler();

	for(uint32_t i = 0; i < Constants::c_maxTextures; ++i) {
	    VkDescriptorImageInfo& imgInfo = imgInfos[i];
	    imgInfo.imageLayout            = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	    imgInfo.imageView              = imgViews[i];
	    imgInfo.sampler                = sampler;
	}

	ClearStruct(writeSet);
	writeSet.dstBinding      = 0;
	writeSet.dstArrayElement = 0;
	writeSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeSet.descriptorCount = Constants::c_maxTextures;
	writeSet.pImageInfo      = imgInfos.data();

	VkPushDescriptorSetInfo pushDescriptor;
	ClearStruct(pushDescriptor);

	vkCmdPushDescriptorSet(a_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeLineLayout, 0, 1, &writeSet);
	*/
}