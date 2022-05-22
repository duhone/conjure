module;

#include "core/Log.h"

export module CR.Engine.Audio.Services;

import CR.Engine.Core;

import<optional>;

namespace CR::Engine::Audio {
	export void ServicesStart();
	export void ServicesStop();

	[[nodiscard]] std::optional<CR::Engine::Core::ServiceLocator>& GetServices();

	export template<typename T, typename... ArgsT>
	void AddService(ArgsT&&... args) {
		CR_ASSERT(GetServices().has_value(), "Audio Services missing");
		GetServices()->Add<T>(std::forward<ArgsT>(args)...);
	}

	export template<typename T>
	T& GetService() {
		CR_ASSERT_AUDIT(GetServices().has_value(), "Audio Services missing");
		return GetServices()->Get<T>();
	}

}    // namespace CR::Engine::Audio

module : private;

namespace cea = CR::Engine::Audio;

[[nodiscard]] std::optional<CR::Engine::Core::ServiceLocator>& cea::GetServices() {
	static std::optional<CR::Engine::Core::ServiceLocator> services;
	return services;
}

void cea::ServicesStart() {
	GetServices().emplace();
}

void cea::ServicesStop() {
	GetServices().reset();
}