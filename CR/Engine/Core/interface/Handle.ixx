module;

#include <core/Log.h>

export module CR.Engine.Core.Handle;

import <cstdint>;
import <utility>;

namespace CR::Engine::Core {
	export template<typename CRTP>
	class Handle {
	  public:
		Handle() = default;
		explicit Handle(uint16_t a_id) { m_id = a_id; }
		~Handle()             = default;
		Handle(const Handle&) = delete;
		Handle(Handle&& a_other) noexcept;
		Handle& operator=(const Handle&) = delete;
		Handle& operator=(Handle&& a_other) noexcept;

		uint16_t asInt() const { return m_id; }

	  protected:
		inline static constexpr uint16_t c_unused{0xffff};

		uint16_t m_id = c_unused;
	};
}    // namespace CR::Engine::Core

template<typename CRTP>
inline CR::Engine::Core::Handle<CRTP>::Handle(Handle&& a_other) noexcept {
	*this = std::move(a_other);
}

template<typename CRTP>
inline CR::Engine::Core::Handle<CRTP>& CR::Engine::Core::Handle<CRTP>::operator=(Handle&& a_other) noexcept {
	m_id         = a_other.m_id;
	a_other.m_id = c_unused;

	return *this;
}
