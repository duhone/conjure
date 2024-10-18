export module CR.Engine.Graphics.Constants;

import <cstdint>;

export namespace CR::Engine::Graphics::Constants {
	// 2 "should" be enough with our current latency gaurantees.
	inline constexpr uint32_t c_maxFramesInFlight = 2;
	inline constexpr uint32_t c_maxTextureSets    = 4;
	// can be larger if needed. just have to have some max.
	inline constexpr uint32_t c_maxTexturesInASet = 256;
	inline constexpr int32_t c_maxTextures        = c_maxTexturesInASet * c_maxTextureSets;
}    // namespace CR::Engine::Graphics::Constants
