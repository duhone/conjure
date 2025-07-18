module;

#include <core/Log.h>

export module CR.Engine.Core.Handle;

import std;
import std.compat;

export namespace CR::Engine::Core {
	// TAG is just there to force a unique type, can just make a new type inline if needed. i.e.
	// Handle<class HandleTag>
	template<typename TAG>
	class Handle {
	  public:
		constexpr Handle() noexcept = default;
		// always take uint64_t, convenience, avoid some casting,
		constexpr explicit Handle(uint64_t a_id, uint64_t a_generation) noexcept {
			CR_ASSERT(a_id < std::numeric_limits<uint16_t>::max(), "handle id's must be less than 64K");
			CR_ASSERT(m_generation < std::numeric_limits<uint16_t>::max(),
			          "handle generation's must be less than 64K");
			m_id         = (uint16_t)a_id;
			m_generation = a_generation;
		}
		// always take uint64_t, convenience, avoid some casting, defaults to generation 0.
		constexpr explicit Handle(uint64_t a_id) noexcept {
			CR_ASSERT(a_id < std::numeric_limits<uint16_t>::max(), "handle id's must be less than 64K");
			m_id         = (uint16_t)a_id;
			m_generation = 0;
		}

		constexpr ~Handle() noexcept              = default;
		Handle(const Handle&) noexcept            = default;
		Handle(Handle&&) noexcept                 = default;
		Handle& operator=(const Handle&) noexcept = default;
		Handle& operator=(Handle&&) noexcept      = default;

		constexpr bool operator==(const Handle&) const noexcept = default;

		// allow implicit cast to int, for indexing into containers tersely.
		constexpr operator uint16_t() const noexcept { return m_id; }
		constexpr bool isValid() const noexcept { return m_id != c_unused; }

		constexpr uint16_t getGeneration() const noexcept { return m_generation; }
		constexpr void incGeneration() noexcept { ++m_generation; }

	  protected:
		inline static constexpr uint16_t c_unused{std::numeric_limits<uint16_t>::max()};

		uint16_t m_id{c_unused};
		uint16_t m_generation{};
	};
}    // namespace CR::Engine::Core
