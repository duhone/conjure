module;

#include "core/Core.h"

#include "Core.h"

export module CR.Engine.Graphics.Commands;

import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Utils;

import CR.Engine.Core;

import std;
import std.compat;

export namespace CR::Engine::Graphics::Commands {
	void RenderPassBegin(VkCommandBuffer& a_cmdBuffer, VkImageView a_colorView, VkImageView a_resolveView,
	                     glm::ivec2 a_windowSize, std::optional<glm::vec4> a_clearColor);
	void RenderPassEnd(VkCommandBuffer& a_cmdBuffer);
	void TransitionToDst(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image, uint32_t a_layerCount);
	void CopyBufferToImg(VkCommandBuffer& a_cmdBuffer, const VkBuffer& a_buffer, VkImage& a_image,
	                     std::span<VkBufferImageCopy> a_copies);
	void TransitionToGraphicsQueue(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image,
	                               uint32_t a_layerCount);
	void TransitionFromTransferQueue(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image,
	                                 uint32_t a_layerCount);

	void TransitionToReadOptimal(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image, uint32_t a_layerCount);
	void TransitionColorAttachToOptimal(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image);
	void TransitionColorAttachToPresent(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image);
}    // namespace CR::Engine::Graphics::Commands

module :private;

namespace cegraph = CR::Engine::Graphics;

void cegraph::Commands::RenderPassBegin(VkCommandBuffer& a_cmdBuffer, VkImageView a_colorView,
                                        VkImageView a_resolveView, glm::ivec2 a_windowSize,
                                        std::optional<glm::vec4> a_clearColor) {
	VkRenderingInfo renderingInfo;
	ClearStruct(renderingInfo);
	renderingInfo.renderArea.extent.width  = a_windowSize.x;
	renderingInfo.renderArea.extent.height = a_windowSize.y;

	renderingInfo.layerCount           = 1;
	renderingInfo.colorAttachmentCount = 1;

	VkRenderingAttachmentInfo colorAttachment;
	ClearStruct(colorAttachment);
	if(a_clearColor.has_value()) {
		colorAttachment.clearValue.color.float32[0] = a_clearColor.value().r;
		colorAttachment.clearValue.color.float32[1] = a_clearColor.value().g;
		colorAttachment.clearValue.color.float32[2] = a_clearColor.value().b;
		colorAttachment.clearValue.color.float32[3] = a_clearColor.value().a;
		colorAttachment.loadOp                      = VK_ATTACHMENT_LOAD_OP_CLEAR;

	} else {
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
	colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.imageView   = a_colorView;
	// TODO: has to be this way for non tiled, but slow for tiled. Fix
	colorAttachment.storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.resolveMode        = VK_RESOLVE_MODE_AVERAGE_BIT;
	colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.resolveImageView   = a_resolveView;

	renderingInfo.pColorAttachments = &colorAttachment;

	vkCmdBeginRendering(a_cmdBuffer, &renderingInfo);
}

void cegraph::Commands::RenderPassEnd(VkCommandBuffer& a_cmdBuffer) {
	vkCmdEndRendering(a_cmdBuffer);
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
	                       (uint32_t)a_copies.size(), a_copies.data());
}

void cegraph::Commands::TransitionToGraphicsQueue(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image,
                                                  uint32_t a_layerCount) {
	VkImageMemoryBarrier barrier;
	ClearStruct(barrier);
	barrier.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask                   = VK_ACCESS_NONE;
	barrier.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex             = GetContext().TransferQueueIndex;
	barrier.dstQueueFamilyIndex             = GetContext().GraphicsQueueIndex;
	barrier.image                           = a_image;
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = a_layerCount;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;

	vkCmdPipelineBarrier(a_cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0,
	                     0, nullptr, 0, nullptr, 1, &barrier);
}

void cegraph::Commands::TransitionFromTransferQueue(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image,
                                                    uint32_t a_layerCount) {
	VkImageMemoryBarrier barrier;
	ClearStruct(barrier);
	barrier.srcAccessMask                   = VK_ACCESS_NONE;
	barrier.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex             = GetContext().TransferQueueIndex;
	barrier.dstQueueFamilyIndex             = GetContext().GraphicsQueueIndex;
	barrier.image                           = a_image;
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = a_layerCount;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;

	vkCmdPipelineBarrier(a_cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
	                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void cegraph::Commands::TransitionToReadOptimal(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image,
                                                uint32_t a_layerCount) {
	VkImageMemoryBarrier barrier;
	ClearStruct(barrier);
	barrier.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.image                           = a_image;
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = a_layerCount;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;

	vkCmdPipelineBarrier(a_cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	                     0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void cegraph::Commands::TransitionColorAttachToOptimal(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image) {
	VkImageMemoryBarrier barrier;
	ClearStruct(barrier);
	barrier.srcAccessMask                   = 0;
	barrier.dstAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.image                           = a_image;
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;

	vkCmdPipelineBarrier(a_cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1,
	                     &barrier);
}

void cegraph::Commands::TransitionColorAttachToPresent(VkCommandBuffer& a_cmdBuffer, const VkImage& a_image) {
	VkImageMemoryBarrier barrier;
	ClearStruct(barrier);
	barrier.srcAccessMask                   = 0;
	barrier.dstAccessMask                   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout                       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout                       = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	barrier.image                           = a_image;
	barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount     = 1;
	barrier.subresourceRange.baseMipLevel   = 0;
	barrier.subresourceRange.levelCount     = 1;

	vkCmdPipelineBarrier(a_cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
	                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1,
	                     &barrier);
}