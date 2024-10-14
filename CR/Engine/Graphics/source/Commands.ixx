module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.Commands;

import CR.Engine.Core;
import CR.Engine.Graphics.Utils;

import <optional>;
import <span>;

namespace CR::Engine::Graphics::Commands {
	export void RenderPassBegin(VkCommandBuffer& a_cmdBuffer, VkRenderPass a_renderPass,
	                            VkFramebuffer a_frameBuffer, glm::ivec2 a_windowSize,
	                            std::optional<glm::vec4> a_clearColor);
	export void RenderPassEnd(VkCommandBuffer& a_cmdBuffer);
	export void TransitionToDst(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image, uint32_t a_layerCount);
	export void CopyBufferToImg(VkCommandBuffer& a_cmdBuffer, const VkBuffer& a_buffer, VkImage& a_image,
	                            std::span<VkBufferImageCopy> a_copies);
}    // namespace CR::Engine::Graphics::Commands

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
		clearValue.color.float32[0]    = a_clearColor.value().r;
		clearValue.color.float32[1]    = a_clearColor.value().g;
		clearValue.color.float32[2]    = a_clearColor.value().b;
		clearValue.color.float32[3]    = a_clearColor.value().a;
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

void cegraph::Commands::TransitionToDst(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image,
                                        uint32_t a_layerCount) {
	VkImageMemoryBarrier barrier;
	ClearStruct(barrier);
	barrier.srcAccessMask                   = VK_ACCESS_NONE;
	barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.image                           = a_image;
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = a_layerCount;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;

	vkCmdPipelineBarrier(a_cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
	                     nullptr, 0, nullptr, 1, &barrier);
}

void cegraph::Commands::CopyBufferToImg(VkCommandBuffer& a_cmdBuffer, const VkBuffer& a_buffer,
                                        VkImage& a_image, std::span<VkBufferImageCopy> a_copies) {
	vkCmdCopyBufferToImage(a_cmdBuffer, a_buffer, a_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                       a_copies.size(), a_copies.data());
}