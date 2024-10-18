module;

#include "Core.h"

#include <glm/glm.hpp>

export module CR.Engine.Graphics.Formats;

export namespace CR::Engine::Graphics::Formats {

	[[nodiscard]] inline VkFormat GetVKFormat(const float&) noexcept {
		return VK_FORMAT_R32_SFLOAT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::vec2&) noexcept {
		return VK_FORMAT_R32G32_SFLOAT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::vec3&) noexcept {
		return VK_FORMAT_R32G32B32_SFLOAT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::vec4&) noexcept {
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i8&) noexcept {
		return VK_FORMAT_R8_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i8vec2&) noexcept {
		return VK_FORMAT_R8G8_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i8vec3&) noexcept {
		return VK_FORMAT_R8G8B8_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i8vec4&) noexcept {
		return VK_FORMAT_R8G8B8A8_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i16&) noexcept {
		return VK_FORMAT_R16_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i16vec2&) noexcept {
		return VK_FORMAT_R16G16_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i16vec3&) noexcept {
		return VK_FORMAT_R16G16B16_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i16vec4&) noexcept {
		return VK_FORMAT_R16G16B16A16_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i32&) noexcept {
		return VK_FORMAT_R32_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i32vec2&) noexcept {
		return VK_FORMAT_R32G32_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i32vec3&) noexcept {
		return VK_FORMAT_R32G32B32_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::i32vec4&) noexcept {
		return VK_FORMAT_R32G32B32A32_SINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u8&) noexcept {
		return VK_FORMAT_R8_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u8vec2&) noexcept {
		return VK_FORMAT_R8G8_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u8vec3&) noexcept {
		return VK_FORMAT_R8G8B8_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u8vec4&) noexcept {
		return VK_FORMAT_R8G8B8A8_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u16&) noexcept {
		return VK_FORMAT_R16G16_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u16vec2&) noexcept {
		return VK_FORMAT_R16G16_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u16vec3&) noexcept {
		return VK_FORMAT_R16G16B16_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u16vec4&) noexcept {
		return VK_FORMAT_R16G16B16A16_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u32&) noexcept {
		return VK_FORMAT_R32_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u32vec2&) noexcept {
		return VK_FORMAT_R32G32_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u32vec3&) noexcept {
		return VK_FORMAT_R32G32B32_UINT;
	}

	[[nodiscard]] inline VkFormat GetVKFormat(const glm::u32vec4&) noexcept {
		return VK_FORMAT_R32G32B32A32_UINT;
	}
}    // namespace CR::Engine::Graphics::Formats

module :private;
