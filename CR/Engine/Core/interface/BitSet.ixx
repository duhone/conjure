module;

#include "core/Log.h"

export module CR.Engine.Core.BitSet;

import <bit>;
import <cstdint>;
import <limits>;
import <cstdio>;
import <array>;

namespace CR::Engine::Core {
	export template<std::uint16_t Size>
	class BitSet final {
		static_assert(Size % 64 == 0, "BitSet must be a multiple of 64 bits, keep it simple");

	  public:
		constexpr BitSet()                         = default;
		constexpr BitSet(const BitSet&)            = default;
		constexpr BitSet(BitSet&&)                 = default;
		constexpr BitSet& operator=(const BitSet&) = default;
		constexpr BitSet& operator=(BitSet&&)      = default;

		// number of integers in the set. i.e. the popcount of entire BitSet
		[[nodiscard]] constexpr uint16_t size() const noexcept {
			uint32_t result{};
			for(const auto& word : m_words) { result += std::popcount(word); }
			CR_ASSERT_AUDIT(result <= std::numeric_limits<uint16_t>::max(), "Bit set supports max of 64K");
			return static_cast<uint16_t>(result);
		}

		[[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

		[[nodiscard]] constexpr uint16_t capacity() const noexcept { return Size; }

		[[nodiscard]] constexpr bool contains(std::uint16_t a_value) noexcept {
			CR_ASSERT_AUDIT(a_value < Size, "bitset capacity not large enough to hold {}", a_value);
			auto& word = m_words[a_value / 64];
			return ((1ull << (a_value % 64)) & word) != 0ull;
		}

		constexpr void insert(std::uint16_t a_value) noexcept {
			CR_ASSERT_AUDIT(a_value < Size, "bitset capacity not large enough to hold {}", a_value);
			auto& word = m_words[a_value / 64];
			word |= 1ull << (a_value % 64);
		}

		constexpr void erase(std::uint16_t a_value) noexcept {
			CR_ASSERT_AUDIT(a_value < Size, "bitset capacity not large enough to hold {}", a_value);
			auto& word = m_words[a_value / 64];
			word &= ~(1ull << (a_value % 64));
		}

		constexpr void clear() noexcept { m_words.fill(0); }

	  private:
		// failing to compile as a std::array for some reason
		std::array<std::uint64_t, Size / 64> m_words{};
	};
}    // namespace CR::Engine::Core

module :private;

namespace cecore = CR::Engine::Core;
