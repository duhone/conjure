module;

#include <core/Log.h>

export module CR.Engine.Core.Handle;

import<cstdint>;
import<utility>;

namespace CR::Engine::Core {
	export template<typename CRTP>
	class Handle {
	  public:
		Handle() = default;
		~Handle();
		Handle(const Handle&) = delete;
		Handle(Handle&& a_other) noexcept;
		Handle& operator=(const Handle&) = delete;
		Handle& operator                 =(Handle&& a_other) noexcept;

	  protected:
		inline static constexpr uint16_t c_unused{0xffff};

		uint16_t m_index = c_unused;
	};
}    // namespace CR::Engine::Core

template<typename CRTP>
inline CR::Engine::Core::Handle<CRTP>::~Handle() {
	if(m_index != c_unused) { static_cast<CRTP*>(this)->FreeHandle(); }
}

template<typename CRTP>
inline CR::Engine::Core::Handle<CRTP>::Handle(Handle&& a_other) noexcept {
	*this = std::move(a_other);
}

template<typename CRTP>
inline CR::Engine::Core::Handle<CRTP>& CR::Engine::Core::Handle<CRTP>::operator=(Handle&& a_other) noexcept {
	if(m_index != c_unused) { static_cast<CRTP*>(this)->FreeHandle(); }
	m_index         = a_other.m_index;
	a_other.m_index = c_unused;

	return *this;
}
