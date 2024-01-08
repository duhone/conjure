module;

#include <core/Log.h>

export module CR.Engine.Core.Handle;

import <cstdint>;
import <limits>;
import <concepts>;

namespace CR::Engine::Core {
	// TAG is just there to force a unique type, can just make a new type inline if needed. i.e.
	// Handle<uint16_t, class HandleTag>
	export template<std::unsigned_integral IDType, typename TAG>
	class Handle {
	  public:
		constexpr Handle() = default;
		// always take uint64_t, convienence, avoid some casting
		constexpr explicit Handle(uint64_t a_id) {
			CR_ASSERT(a_id < c_unused, "this Handle type can hold a value this large {}", a_id);
			m_id = (IDType)a_id;
		}
		constexpr ~Handle()                       = default;
		Handle(const Handle&) noexcept            = default;
		Handle(Handle&&) noexcept                 = default;
		Handle& operator=(const Handle&) noexcept = default;
		Handle& operator=(Handle&&) noexcept      = default;

		constexpr uint16_t asInt() const { return m_id; }
		constexpr bool isValid() const { return m_id != c_unused; }

	  protected:
		inline static constexpr IDType c_unused{std::numeric_limits<IDType>::max()};

		IDType m_id = c_unused;
	};
}    // namespace CR::Engine::Core
