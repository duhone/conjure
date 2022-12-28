export module CR.Engine.Core.EightCC;

import <cassert>;
import <cstdint>;

export namespace CR::Engine::Core {
	consteval std::uint64_t EightCC(const char eightcc[9]) {
		std::uint64_t result{};
		for(int i = 0; i < 8; ++i) {
			//assert(eightcc[i] >= 32 && eightcc[i] <= 126);
			result <<= 8;
			result |= eightcc[i];
		}

		return result;
	}
}    // namespace CR::Engine::Core
