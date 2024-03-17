module;

#include "core/Log.h"

export module CR.Engine.Core.BitSet;

import <array>;
import <bit>;
import <cstdint>;
import <cstdio>;
import <iterator>;
import <limits>;

namespace CR::Engine::Core {
	// std::bitset is missing some fundamental opearations, making it slow in practice.
	// This is basically a std::bitset, except it can iterator 1 bits fast, and find the first 0 bit fast.
	// The API is different that std::bitset, its treated more as a set of integers. The API follows std::set
	// instead. The Size template parameter reprents the largest integer that the set can contain + 1. i.e.
	// Size 512 means this set can hold integers from 0 to 511. Size must be a multiple of 64. Multiples of
	// 512 may make more sense in practice, since you can fit 512 bits in a cache line.
	//
	// The current implementation is the fastest for sets with Size <= 2048 or so. Could use an alternate
	// implemtation for Sizes 2049-16384; Basically needs to burn one cache line for some simple acceleration
	// data, current size(), min/max contained value, ect. And maybe switch the bitset itself to a heap
	// alocation. Larger than 16384 probably faster to use something like roaring bitmap. With that in mind,
	// this class has a hard cap at 64K.
	export template<std::uint16_t Size>
	class BitSet final {
		static_assert(Size % 64 == 0, "BitSet must be a multiple of 64 bits, keep it simple");
		constexpr inline static std::uint16_t c_endIDX = (Size / 64);

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

		// insert a_count integers starting at a_first
		constexpr void insertRange(std::uint16_t a_first, std::uint16_t a_count) noexcept {
			CR_ASSERT_AUDIT((a_first + a_count) <= Size,
			                "bitset capacity not large enough to hold integers from {} to {}", a_first,
			                (a_first + a_count));
			uint64_t currentWord = a_first / 64;
			uint16_t numFirst    = a_first % 64;
			if(numFirst > 0) {
				a_count -= numFirst;
				uint64_t numFirstMask = ~(~uint64_t(0) >> (64 - numFirst));
				auto& word            = m_words[currentWord];
				word |= numFirstMask;
				++currentWord;
			}
			uint64_t numWords = a_count / 64;
			while(numWords > 0) {
				m_words[currentWord] = ~uint64_t(0);
				++currentWord;
			}
			uint64_t numLast = a_count % 64;
			if(numLast > 0) {
				uint64_t numLastMask = (~uint64_t(0) >> (64 - numFirst));
				auto& word           = m_words[currentWord];
				word |= numLastMask;
			}
		}

		constexpr void erase(std::uint16_t a_value) noexcept {
			CR_ASSERT_AUDIT(a_value < Size, "bitset capacity not large enough to hold {}", a_value);
			auto& word = m_words[a_value / 64];
			word &= ~(1ull << (a_value % 64));
		}

		constexpr void clear() noexcept { m_words.fill(0); }

		// TODO: I don't like the name of this function.
		// Find an integer not in the set. Will be the smallest one
		constexpr std::uint16_t FindNotInSet() const noexcept {
			CR_ASSERT_AUDIT(size() != capacity(), "bit set is full, undefined behavior");
			int32_t i{};
			std::uint64_t word{};
			while((word = m_words[i++]) == std::numeric_limits<std::uint64_t>::max()) {}
			auto bitPos = word == 0 ? 0 : std::countr_one(word);
			return static_cast<std::uint16_t>(bitPos + (64 * (i - 1)));
		}

		class ConstIterator {
			friend BitSet;

		  public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type   = void;
			using value_type        = std::uint16_t;
			using pointer           = const value_type* const;
			using reference         = const value_type&;

			ConstIterator()                                    = delete;
			~ConstIterator()                                   = default;
			ConstIterator(const ConstIterator&)                = default;
			ConstIterator(ConstIterator&&) noexcept            = default;
			ConstIterator& operator=(const ConstIterator&)     = default;
			ConstIterator& operator=(ConstIterator&&) noexcept = default;

			reference operator*() { return m_value; }
			pointer operator->() { return &m_value; }

			std::uint16_t& operator++() {
				while(m_word == 0 && m_wordIdx != c_endIDX) {
					++m_wordIdx;
					if(m_wordIdx != c_endIDX) {
						m_word = m_bitset.m_words[m_wordIdx];
					} else {
						m_word  = 0;
						m_value = 0;
						return m_value;
					}
				}
				CR_ASSERT_AUDIT(m_word != 0, "logic error, m_word should never be 0 here");
				auto bitPos = std::countr_zero(m_word);
				CR_ASSERT_AUDIT(bitPos + m_wordIdx * 64 <= std::numeric_limits<std::uint16_t>::max(),
				                "logic error, impossible value");
				m_value                = static_cast<std::uint16_t>(bitPos + m_wordIdx * 64);
				std::uint16_t bitClear = ~(1 << bitPos);
				m_word &= bitClear;
				return m_value;
			}
			std::uint16_t operator++(int) {
				auto result = m_value;
				++(*this);
				return result;
			}

			bool operator==(const ConstIterator& a_other) const {
				CR_ASSERT_AUDIT(&m_bitset == &a_other.m_bitset, "comparing iterators from different bitsets");
				if(m_wordIdx != a_other.m_wordIdx) { return false; }
				if(m_word != a_other.m_word) { return false; }
				return true;
			}
			bool operator!=(const ConstIterator& a_other) const = default;

		  private:
			ConstIterator(const BitSet& a_bitset) : m_bitset(a_bitset) {
				m_word = m_bitset.m_words[m_wordIdx];
				++(*this);
			}
			ConstIterator(const BitSet& a_bitset, bool) : m_bitset(a_bitset) { m_wordIdx = c_endIDX; }

			const BitSet& m_bitset;
			std::uint64_t m_word{};
			std::uint16_t m_wordIdx{};
			std::uint16_t m_value{};
		};
		// Not sure why we aren't satisfying forward_iterator requirements?
		// static_assert(std::forward_iterator<ConstIterator>);

		ConstIterator begin() const { return ConstIterator{*this}; }
		ConstIterator end() const { return ConstIterator{*this, true}; }
		ConstIterator cbegin() const { return ConstIterator{*this}; }
		ConstIterator cend() const { return ConstIterator{*this, true}; }

	  private:
		// failing to compile as a std::array for some reason
		std::array<std::uint64_t, Size / 64> m_words{};
	};
}    // namespace CR::Engine::Core

module :private;

namespace cecore = CR::Engine::Core;
