﻿export module CR.Engine.Graphics.Constants;

import <cstdint>;

export namespace CR::Engine::Graphics::Constants {
	inline constexpr uint32_t c_maxTextureSets = 4;
	// can be larger if needed. just have to have some max.
	inline constexpr uint32_t c_maxTexturesInASet = 256;
	inline constexpr int32_t c_maxTextures        = c_maxTexturesInASet * c_maxTextureSets;
}    // namespace CR::Engine::Graphics::Constants
