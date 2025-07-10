module;

#include "core/Log.h"

export module CR.Engine.Core.Services;

import CR.Engine.Core.ServiceLocator;

import std;

namespace CR::Engine::Core {
	export void ServicesStart();
	export void ServicesStop();

	[[nodiscard]] std::optional<CR::Engine::Core::ServiceLocator>& GetServices();

	export template<typename T, typename... ArgsT>
	T& AddService(ArgsT&&... args) {
		CR_ASSERT(GetServices().has_value(), "Services missing");
		return GetServices()->Add<T>(std::forward<ArgsT>(args)...);
	}

	export template<typename T>
	T& GetService() {
		CR_ASSERT_AUDIT(GetServices().has_value(), "Services missing");
		return GetServices()->Get<T>();
	}

}    // namespace CR::Engine::Core

module :private;

namespace cecore = CR::Engine::Core;

[[nodiscard]] std::optional<CR::Engine::Core::ServiceLocator>& cecore::GetServices() {
	static std::optional<CR::Engine::Core::ServiceLocator> services;
	return services;
}

void cecore::ServicesStart() {
	GetServices().emplace();
}

void cecore::ServicesStop() {
	GetServices().reset();
}