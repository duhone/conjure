export module CR.Engine.Core.Hash;

import std;
import std.compat;

// FNV1-a hash https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
// xxHash, spooky hack, city hash, might be better choices. This one is simple and easy to make compile time
// though. It operates one byte at a time, so is not particularly fast. Current requirement is for both
// runtime and compile time to produce exactly the same result. Good enough for now.

namespace CR::Engine::Core {
	export inline constexpr uint32_t Hash32(std::ranges::range auto a_data) {
		constexpr uint32_t c_offsetBias = 0x811c9dc5;
		constexpr uint32_t c_prime      = 0x01000193;

		uint32_t hash = c_offsetBias;

		for(const auto& ch : a_data) {
			static_assert(sizeof(ch) == 1, "Hash only works with bytes currently");
			hash ^= ch;
			hash *= c_prime;
		}

		return hash;
	}

	export inline consteval uint32_t C_Hash32(std::ranges::range auto a_data) {
		return Hash32(a_data);
	}

	export inline consteval uint32_t C_Hash32(std::string_view a_data) {
		return Hash32(a_data);
	}

	export inline constexpr uint64_t Hash64(std::ranges::range auto a_data) {
		constexpr uint64_t c_offsetBias = 0xcbf29ce484222325ull;
		constexpr uint64_t c_prime      = 0x00000100000001B3ull;

		uint64_t hash = c_offsetBias;

		for(const auto& ch : a_data) {
			static_assert(sizeof(ch) == 1, "Hash only works with bytes currently");
			hash ^= ch;
			hash *= c_prime;
		}

		return hash;
	}

	export inline constexpr uint64_t Hash64(const char* a_data) {
		constexpr uint64_t c_offsetBias = 0xcbf29ce484222325ull;
		constexpr uint64_t c_prime      = 0x00000100000001B3ull;

		uint64_t hash = c_offsetBias;

		static_assert(sizeof(a_data[0]) == 1, "Hash only works with bytes currently");
		int i = 0;
		while(a_data[i] != 0) {
			hash ^= a_data[i++];
			hash *= c_prime;
		}

		return hash;
	}

	export inline consteval uint64_t C_Hash64(std::ranges::range auto a_data) {
		return Hash64(a_data);
	}

	export inline consteval uint64_t C_Hash64(std::string_view a_data) {
		return Hash64(a_data);
	}
}    // namespace CR::Engine::Core
