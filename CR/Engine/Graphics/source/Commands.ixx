module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.Commands;

import CR.Engine.Core;
import CR.Engine.Graphics.Utils;

import <optional>;

namespace CR::Engine::Graphics::Commands {
	export void RenderPassBegin(VkCommandBuffer& a_cmdBuffer, VkRenderPass a_renderPass,
	                     VkFramebuffer a_frameBuffer, glm::ivec2 a_windowSize, std::optional<glm::vec4> a_clearColor);
	export void RenderPassEnd(VkCommandBuffer& a_cmdBuffer);
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

void cegraph::Commands::RenderPassBegin(VkCommandBuffer& a_cmdBuffer, VkRenderPass a_renderPass,
                                        VkFramebuffer a_frameBuffer, glm::ivec2 a_windowSize,
                                        std::optional<glm::vec4> a_clearColor) {
	VkRenderPassBeginInfo renderPassInfo;
	ClearStruct(renderPassInfo);
	renderPassInfo.renderPass               = a_renderPass;
	renderPassInfo.renderArea.extent.width  = a_windowSize.x;
	renderPassInfo.renderArea.extent.height = a_windowSize.y;
	renderPassInfo.framebuffer              = a_frameBuffer;
	VkClearValue clearValue;
	if(a_clearColor.has_value()) {
		clearValue.color.float32[0] = a_clearColor.value().r;
		clearValue.color.float32[1] = a_clearColor.value().g;
		clearValue.color.float32[2] = a_clearColor.value().b;
		clearValue.color.float32[3] = a_clearColor.value().a;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues    = &clearValue;

	} else {
		renderPassInfo.clearValueCount = 0;
	}

	vkCmdBeginRenderPass(a_cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void cegraph::Commands::RenderPassEnd(VkCommandBuffer& a_cmdBuffer) {
	vkCmdEndRenderPass(a_cmdBuffer);
}