export module CR.Engine.Core.EightCC;

import <cassert>;
import <cstdint>;
import <cstdio>;
import <string_view>;

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

	//only use in debugging/logging code
	std::string_view EightCC(std::uint64_t eightcc) {
		return {reinterpret_cast<const char*>(&eightcc), 8};
	}
}    // namespace CR::Engine::Core
