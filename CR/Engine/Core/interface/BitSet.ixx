module;

#include "core/Log.h"

export module CR.Engine.Core.BitSet;

import <bit>;
import <cstdint>;
import <limits>;
import <vector>;

namespace CR::Engine::Core {
	export class alignas(64) BitSet final {
		static inline const uint32_t c_numStaticWords = 4;

	  public:
		BitSet()                         = default;
		BitSet(const BitSet&)            = default;
		BitSet(BitSet&&)                 = default;
		BitSet& operator=(const BitSet&) = default;
		BitSet& operator=(BitSet&&)      = default;

		[[nodiscard]] uint16_t size() const noexcept {
			uint32_t result{};
			for(const auto& word : m_staticWords) { result += std::popcount(word); }
			for(const auto& word : m_dynamicWords) { result += std::popcount(word); }
			CR_ASSERT_AUDIT(result <= std::numeric_limits<uint16_t>::max(), "Bit set supports max of 64K");
			return static_cast<uint16_t>(result);
		}

		[[nodiscard]] uint16_t capacity() const noexcept {
			auto result = (std::size(m_staticWords) + std::size(m_dynamicWords)) * 64;
			CR_ASSERT_AUDIT(result <= std::numeric_limits<uint16_t>::max(), "Bit set supports max of 64K");
			return static_cast<uint16_t>(result);
		}

	  private:
		// failing to compile as a std::array for some reason
		std::uint64_t m_staticWords[c_numStaticWords];
		std::vector<uint64_t> m_dynamicWords;
	};

	static_assert(sizeof(BitSet) <= 64, "A Bitset should only take 1 cache line in-situ");
}    // namespace CR::Engine::Core

module :private;

namespace cecore = CR::Engine::Core;
