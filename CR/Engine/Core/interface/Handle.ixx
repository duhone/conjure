module;

#include <core/Log.h>

export module CR.Engine.Core.Handle;

import <cstdint>;
import <limits>;
import <concepts>;

namespace CR::Engine::Core {
	// TAG is just there to force a unique type, can just make a new type inline if needed. i.e.
	// Handle<class HandleTag>
	export template<typename TAG>
	class Handle {
	  public:
		constexpr Handle() = default;
		// always take uint64_t, convenience, avoid some casting
		constexpr explicit Handle(uint64_t a_id) {
			CR_ASSERT(a_id < c_unused, "this Handle type can hold a value this large {}", a_id);
			m_id = (uint16_t)a_id;
		}
		constexpr ~Handle()                       = default;
		Handle(const Handle&) noexcept            = default;
		Handle(Handle&&) noexcept                 = default;
		Handle& operator=(const Handle&) noexcept = default;
		Handle& operator=(Handle&&) noexcept      = default;

		constexpr uint16_t asInt() const { return m_id; }
		constexpr bool isValid() const { return m_id != c_unused; }

	  protected:
		inline static constexpr uint16_t c_unused{std::numeric_limits<uint16_t>::max()};

		uint16_t m_id = c_unused;
	};

	// Similar to Handle, but will decrement a reference count when destroyed. Can be moved, but not copied.
	// Does not delete anything on its own, just decrements a ref count.
	export template<typename TAG>
	class OwningHandle {
	  public:
		constexpr OwningHandle() = default;
		constexpr explicit OwningHandle(Handle<TAG> a_handle, uint16_t* refCount) {
			m_handle   = a_handle;
			m_refCount = refCount;
			(*m_refCount) += 1;
		}
		constexpr ~OwningHandle() {
			if(m_refCount) {
				CR_ASSERT_AUDIT(*m_refCount == 0, "m_refCount already 0, logic error");
				*m_refCount -= 1;
			}
		}
		OwningHandle(const OwningHandle&) noexcept = delete;
		OwningHandle(OwningHandle&& a_other) noexcept { *this = std::move(a_other); }
		OwningHandle& operator=(const OwningHandle&) noexcept = delete;
		OwningHandle& operator=(OwningHandle&& a_other) noexcept {
			if(m_refCount) {
				CR_ASSERT_AUDIT(*m_refCount == 0, "m_refCount already 0, logic error");
				*m_refCount -= 1;
			}
			m_handle           = a_other.m_handle;
			m_refCount         = a_other.m_refCount;
			a_other.m_refCount = nullptr;
			a_other.m_handle   = Handle<TAG>{};
		}

		constexpr Handle<TAG> getHandle() const { return m_handle; }
		constexpr uint16_t asInt() const { return m_handle.asInt(); }
		constexpr bool isValid() const { return m_handle.isValid(); }
		constexpr operator Handle<TAG>() { return m_handle; }

	  protected:
		inline static constexpr uint16_t c_unused{std::numeric_limits<uint16_t>::max()};

		Handle<TAG> m_handle;
		uint16_t* m_refCount{};
	};
}    // namespace CR::Engine::Core
