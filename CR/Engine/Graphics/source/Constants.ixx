export module CR.Engine.Graphics.Constants;

import <cstdint>;

export namespace CR::Engine::Graphics::Constants {
	// 2 "should" be enough with our current latency gaurantees.
	inline constexpr uint32_t c_maxFramesInFlight = 2;
	inline constexpr uint32_t c_maxTextureSets    = 4;
	// can be larger if needed. just have to have some max.
	inline constexpr uint32_t c_maxTexturesInASet = 256;
	inline constexpr int32_t c_maxTextures        = c_maxTexturesInASet * c_maxTextureSets;
	// can handle up to 64K if needed, some things(bitset, ect) probably need some work though. This is total
	// number created in "world", not number rendered, or the number of "templates" in sprites.json.
	inline constexpr int32_t c_maxSprites = 4096;
	// most common monitor refresh rates are evenly divisable by this.
	inline constexpr uint32_t c_designRefreshRate = 720;
}    // namespace CR::Engine::Graphics::Constants
