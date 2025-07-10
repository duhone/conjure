module;

#include "core/Log.h"

export module CR.Engine.Core.Embedded;

import std;

namespace CR::Engine::Core {
	// For when you need to control when construction/destruction happens, but you want	the actual memory to
	// be in-situ. follows std::optional api as appropriate. intended only for member variables.
	export template<typename T>
	class Embedded final {
	  public:
		Embedded() = default;
		~Embedded() {
#if CR_DEBUG || CR_RELEASE
			CR_ASSERT(!m_initialized, "Forgot to reset an Embedded");
#endif
		}

		Embedded(const auto&) = delete;
		Embedded(auto&&)      = delete;

		Embedded& operator=(const auto&) = delete;
		Embedded& operator=(auto&&)      = delete;

		template<typename... ArgsT>
		void emplace(ArgsT&&... a_args) {
			std::construct_at(reinterpret_cast<T*>(m_data), std::forward<ArgsT>(a_args)...);
#if CR_DEBUG || CR_RELEASE
			m_initialized = true;
#endif
		}

		void reset() {
			std::destroy_at(reinterpret_cast<T*>(m_data));
#if CR_DEBUG || CR_RELEASE
			m_initialized = false;
#endif
		}

		T* operator->() { return std::launder(reinterpret_cast<T*>(m_data)); }
		const T* operator->() const { return std::launder(reinterpret_cast<T*>(m_data)); }

		T& operator*() { return *std::launder(reinterpret_cast<T*>(m_data)); }
		const T& operator*() const { return *std::launder(reinterpret_cast<T*>(m_data)); }

	  private:
#if CR_DEBUG || CR_RELEASE
		bool m_initialized{};
#endif
		alignas(T) std::byte m_data[sizeof(T)];
	};
}    // namespace CR::Engine::Core
