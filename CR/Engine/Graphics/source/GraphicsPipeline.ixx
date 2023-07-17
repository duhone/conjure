module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.GraphicsPipeline;

import CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import <vector>;

namespace CR::Engine::Graphics {
	export class GraphicsPipeline {
	  public:
		GraphicsPipeline() = default;
		GraphicsPipeline(VkDevice a_device);
		~GraphicsPipeline();
		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline(GraphicsPipeline&& a_other)         = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&& a_other) = delete;

	  private:
		VkDevice m_device;

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

cegraph::GraphicsPipeline::GraphicsPipeline(VkDevice a_device) : m_device(a_device) {
}

cegraph::GraphicsPipeline::~GraphicsPipeline() {
}
