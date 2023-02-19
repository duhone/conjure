module;

#include "core/Log.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"

export module CR.Engine.Graphics.DeviceService;

import CR.Engine.Core;
import CR.Engine.Platform;

import <string>;
import <typeindex>;
import <vector>;

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

		void Stop();

	  private:
		ceplat::Window& m_window;
		vk::Instance m_instance;
		vk::SurfaceKHR m_primarySurface;
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

cegraph::DeviceService::DeviceService(ceplat::Window& a_window) : m_window(a_window) {
	vk::DynamicLoader loader;
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
	    loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	[[maybe_unused]] vk::ApplicationInfo appInfo;
	appInfo.pApplicationName   = CR_APP_NAME;
	appInfo.applicationVersion = CR_VERSION_APP;
	appInfo.pEngineName        = CR_ENGINE_NAME;
	appInfo.engineVersion      = CR_VERSION_ENGINE;
	appInfo.apiVersion         = VK_API_VERSION_1_2;

	vk::InstanceCreateInfo createInfo;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> extensions;
	extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	createInfo.setPEnabledExtensionNames(extensions);

	m_instance = vk::createInstance(createInfo);
	VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

	vk::Win32SurfaceCreateInfoKHR win32Surface;
	win32Surface.hinstance = reinterpret_cast<HINSTANCE>(m_window.GetHInstance());
	win32Surface.hwnd      = reinterpret_cast<HWND>(m_window.GetHWND());

	m_primarySurface = m_instance.createWin32SurfaceKHR(win32Surface);
}

void cegraph::DeviceService::Stop() {
	m_instance.destroySurfaceKHR(m_primarySurface);
	m_instance.destroy();
}

void cegraph::DeviceService::Update() {}
