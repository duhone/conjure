module;

#include "core/Log.h"

#include "vulkan/vulkan.h"

export module CR.Engine.Graphics.DeviceService;

import CR.Engine.Core;
import CR.Engine.Platform;

import <typeindex>;

namespace cecore = CR::Engine::Core;
namespace ceplat = CR::Engine::Platform;

namespace CR::Engine::Graphics {
	export class DeviceService {
	  public:
		static inline constexpr uint64_t s_typeIndex = CR::Engine::Core::EightCC("EGraDevc");

		DeviceService(ceplat::Window& a_window);
		~DeviceService()                    = default;
		DeviceService(const DeviceService&) = delete;
		DeviceService(DeviceService&&)      = delete;

		DeviceService& operator=(const DeviceService&) = delete;
		DeviceService& operator=(DeviceService&&)      = delete;

		void Update();

	  private:
		ceplat::Window& m_window;
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

cegraph::DeviceService::DeviceService(ceplat::Window& a_window) : m_window(a_window) {
	[[maybe_unused]] VkApplicationInfo appInfo;
	appInfo.pApplicationName   = CR_APP_NAME;
	appInfo.applicationVersion = CR_VERSION_APP;
	appInfo.pEngineName        = CR_ENGINE_NAME;
	appInfo.engineVersion      = CR_VERSION_ENGINE;
	appInfo.apiVersion         = VK_API_VERSION_1_2;
}

void cegraph::DeviceService::Update() {}
