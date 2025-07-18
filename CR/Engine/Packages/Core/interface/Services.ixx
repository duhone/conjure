// Not used at the moment. Note that it isn't currently started in core.ixx, so won't work.
module;

#include "core/Log.h"

export module CR.Engine.Core.Services;

import CR.Engine.Core.ServiceLocator;

import std;

export namespace CR::Engine::Core::Services {
	void Initialize();
	void Shutdown();

	[[nodiscard]] std::optional<CR::Engine::Core::ServiceLocator>& GetServices();

	template<typename T, typename... ArgsT>
	T& AddService(ArgsT&&... args) {
		CR_ASSERT(GetServices().has_value(), "Services missing");
		return GetServices()->Add<T>(std::forward<ArgsT>(args)...);
	}

	template<typename T>
	T& GetService() {
		CR_ASSERT_AUDIT(GetServices().has_value(), "Services missing");
		return GetServices()->Get<T>();
	}

}    // namespace CR::Engine::Core::Services

module :private;

namespace cecore = CR::Engine::Core;

[[nodiscard]] std::optional<CR::Engine::Core::ServiceLocator>& cecore::Services::GetServices() {
	static std::optional<CR::Engine::Core::ServiceLocator> services;
	return services;
}

void cecore::Services::Initialize() {
	GetServices().emplace();
}

void cecore::Services::Shutdown() {
	GetServices().reset();
}