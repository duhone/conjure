module;

#include "generated/graphics/computePipelines_generated.h"

#include "flatbuffers/idl.h"

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

namespace CR::Engine::Graphics {
	export class ComputePipelines {
	  public:
		ComputePipelines() = default;
		ComputePipelines(const Context& a_context);
		~ComputePipelines();
		ComputePipelines(const ComputePipelines&)               = delete;
		ComputePipelines(ComputePipelines&& a_other)            = delete;
		ComputePipelines& operator=(const ComputePipelines&)    = delete;
		ComputePipelines& operator=(ComputePipelines&& a_other) = delete;

		void Update(const Shaders& a_shaders);

		bool IsReady() const { return m_ready.load(std::memory_order_acquire); }

	  private:
		const Context& m_context;

		VkPipelineLayout m_pipeLineLayout;
		VkDescriptorSetLayout m_descriptorSetLayout;
		std::vector<VkPipeline> m_pipelines;

		std::atomic_bool m_ready;
		bool m_startedLoad{};
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace ceplat  = CR::Engine::Platform;
namespace cegraph = CR::Engine::Graphics;

cegraph::ComputePipelines::ComputePipelines(const Context& a_context) : m_context(a_context) {}

void cegraph::ComputePipelines::Update(const Shaders& a_shaders) {
	if(IsReady() || m_startedLoad) { return; }

	if(!a_shaders.IsReady()) { return; }

	m_startedLoad = true;

	GraphicsThread::EnqueueTask(
	    [this, &a_shaders]() {
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
		        vkCreateDescriptorSetLayout(m_context.Device, &dslInfo, nullptr, &m_descriptorSetLayout);
		    CR_ASSERT(result == VK_SUCCESS, "failed to create a descriptor set layout");

		    VkPipelineLayoutCreateInfo layoutInfo;
		    ClearStruct(layoutInfo);
		    layoutInfo.pushConstantRangeCount = 0;
		    layoutInfo.pPushConstantRanges    = nullptr;
		    layoutInfo.setLayoutCount         = 1;
		    layoutInfo.pSetLayouts            = &m_descriptorSetLayout;

		    result = vkCreatePipelineLayout(m_context.Device, &layoutInfo, nullptr, &m_pipeLineLayout);
		    CR_ASSERT(result == VK_SUCCESS, "failed to create a pipeline layout");

		    flatbuffers::Parser parser = assetService.GetData(
		        cecore::C_Hash64("Graphics/computePipelines.json"), SCHEMAS_COMPUTEPIPELINES);

		    auto computePipelines = Flatbuffers::GetComputePipelines(parser.builder_.GetBufferPointer());

		    for(const auto& pipe : *computePipelines->pipelines()) {
			    auto compShader = a_shaders.GetShader(cecore::Hash64(pipe->compute_shader()->c_str()));

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

			    result = vkCreateComputePipelines(m_context.Device, VK_NULL_HANDLE, 1, &pipeInfo, nullptr,
			                                      &m_pipelines.emplace_back());
			    CR_ASSERT(result == VK_SUCCESS, "failed to create a graphics pipeline");
		    }
	    },
	    m_ready);
}

cegraph::ComputePipelines::~ComputePipelines() {
	for(auto& pipeline : m_pipelines) { vkDestroyPipeline(m_context.Device, pipeline, nullptr); }
	vkDestroyPipelineLayout(m_context.Device, m_pipeLineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_context.Device, m_descriptorSetLayout, nullptr);
}
