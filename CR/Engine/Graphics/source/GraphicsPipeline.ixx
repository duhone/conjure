module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.GraphicsPipeline;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Shaders;
import CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import <vector>;

namespace CR::Engine::Graphics {
	export class GraphicsPipeline {
	  public:
		GraphicsPipeline() = default;
		GraphicsPipeline(const Context& a_context, const Shaders& a_shaders, uint64_t a_vertShader,
		                 uint64_t a_fragShader);
		~GraphicsPipeline();
		GraphicsPipeline(const GraphicsPipeline&)               = delete;
		GraphicsPipeline(GraphicsPipeline&& a_other)            = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&)    = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&& a_other) = delete;

	  private:
		const Context& m_context;

		VkPipelineLayout m_pipeLineLayout;
		VkPipeline m_pipeline;
		VkDescriptorSetLayout m_descriptorSetLayout;
		VkSampler m_sampler;

		uint32_t m_lastTextureVersion{0};
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

cegraph::GraphicsPipeline::GraphicsPipeline(const Context& a_context, const Shaders& a_shaders,
                                            uint64_t a_vertShader, uint64_t a_fragShader) :
    m_context(a_context) {
	VkShaderModule vertShader = a_shaders.GetShader(a_vertShader);
	VkShaderModule fragShader = a_shaders.GetShader(a_fragShader);

	VkSpecializationMapEntry fragSpecInfoEntrys;
	fragSpecInfoEntrys.constantID = 0;
	fragSpecInfoEntrys.offset     = 0;
	fragSpecInfoEntrys.size       = sizeof(int32_t);

	VkSpecializationInfo fragSpecInfo;
	fragSpecInfo.dataSize      = sizeof(c_maxTextures);
	fragSpecInfo.pData         = &c_maxTextures;
	fragSpecInfo.mapEntryCount = 1;
	fragSpecInfo.pMapEntries   = &fragSpecInfoEntrys;

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
}

cegraph::GraphicsPipeline::~GraphicsPipeline() {}
