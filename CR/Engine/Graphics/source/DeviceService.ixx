module;

#include "core/Log.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"

export module CR.Engine.Graphics.DeviceService;

import CR.Engine.Core;
import CR.Engine.Platform;

import <optional>;
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
		vk::PhysicalDevice FindDevice();

		ceplat::Window& m_window;
		vk::Instance m_instance;
		vk::SurfaceKHR m_primarySurface;

		vk::Device m_device;
		int32_t m_graphicsQueueIndex{-1};
		int32_t m_transferQueueIndex{-1};
		int32_t m_presentationQueueIndex{-1};
		vk::Queue m_graphicsQueue;
		vk::Queue m_transferQueue;
		vk::Queue m_presentationQueue;
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

	vk::PhysicalDevice selectedDevice = FindDevice();
}

void cegraph::DeviceService::Stop() {
	m_instance.destroySurfaceKHR(m_primarySurface);
	m_instance.destroy();
}

void cegraph::DeviceService::Update() {}

vk::PhysicalDevice cegraph::DeviceService::FindDevice() {
	std::vector<vk::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();

	vk::PhysicalDevice selectedDevice;
	bool foundDevice = false;
	for(auto& device : physicalDevices) {
		auto props = device.getProperties();
		CR_LOG("Device Name : {}", props.deviceName);
		if(props.apiVersion < VK_API_VERSION_1_2) {
			CR_WARN("Device didn't support vulkan 1.2");
			continue;
		}
		auto memProps = device.getMemoryProperties();
		CR_LOG("  Device max allocations: {}", props.limits.maxMemoryAllocationCount);
		CR_LOG("  Device max array layers: {}", props.limits.maxImageArrayLayers);
		CR_LOG("  Device max 2D image dimensions: {}", props.limits.maxImageDimension2D);
		CR_LOG("  Device max multisample: {}", (VkFlags)props.limits.framebufferColorSampleCounts);
		for(uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
			if(memProps.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) {
				CR_LOG("  Device Local Memory Amount: {}MB",
				       memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size / (1024 * 1024));
			}
		}

		std::vector<vk::QueueFamilyProperties> queueProps = device.getQueueFamilyProperties();
		std::vector<uint32_t> graphicsQueues;
		std::vector<uint32_t> transferQueues;
		std::vector<uint32_t> presentationQueues;
		std::optional<uint32_t> dedicatedTransfer;
		std::optional<uint32_t> graphicsAndPresentation;
		for(uint32_t i = 0; i < queueProps.size(); ++i) {
			bool supportsGraphics     = false;
			bool supportsCompute      = false;
			bool supportsTransfer     = false;
			bool supportsPresentation = false;

			CR_LOG("Queue family: {}", i);
			// This one should only be false for tesla compute cards and similiar
			if((queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) && queueProps[i].queueCount >= 1) {
				supportsGraphics = true;
				graphicsQueues.push_back(i);
				CR_LOG("  supports graphics");
			}
			if((queueProps[i].queueFlags & vk::QueueFlagBits::eCompute) && queueProps[i].queueCount >= 1) {
				supportsCompute = true;
				CR_LOG("  supports compute");
			}
			if((queueProps[i].queueFlags & vk::QueueFlagBits::eTransfer) && queueProps[i].queueCount >= 1) {
				supportsTransfer = true;
				transferQueues.push_back(i);
				CR_LOG("  supports transfer");
			}
			if(device.getSurfaceSupportKHR(i, m_primarySurface)) {
				supportsPresentation = true;
				presentationQueues.push_back(i);
				CR_LOG("  supports presentation");
			}

			// for transfers, prefer a dedicated transfer queue(probably doesnt matter). If more than one,
			// grab first
			if(!supportsGraphics && !supportsCompute && !supportsPresentation && supportsTransfer &&
			   !dedicatedTransfer.has_value()) {
				dedicatedTransfer = i;
			}
			// For graphics, we prefer one that does both graphics and presentation
			if(supportsGraphics && supportsPresentation && !graphicsAndPresentation.has_value()) {
				graphicsAndPresentation = i;
			}
		}

		if(dedicatedTransfer.has_value()) {
			m_transferQueueIndex = dedicatedTransfer.value();
		} else if(!transferQueues.empty()) {
			m_transferQueueIndex = transferQueues[0];
		} else {
			CR_LOG("Could not find a valid vulkan transfer queue");
		}

		if(graphicsAndPresentation.has_value()) {
			m_graphicsQueueIndex = m_presentationQueueIndex = graphicsAndPresentation.value();
		} else if(!graphicsQueues.empty() && !presentationQueues.empty()) {
			m_graphicsQueueIndex     = graphicsQueues[0];
			m_presentationQueueIndex = presentationQueues[0];
		} else {
			if(graphicsQueues.empty()) { CR_LOG("Could not find a valid vulkan graphics queue"); }
			if(presentationQueues.empty()) { CR_LOG("Could not find a valid presentation queue"); }
		}

		CR_LOG("graphics queue family: {} transfer queue index: {} presentation queue index: {}",
		       m_graphicsQueueIndex, m_transferQueueIndex, m_presentationQueueIndex);
		auto features = device.getFeatures();

		// TODO: We dont have a good heuristic for selecting a device, for now just take first one that
		// supports graphics and hope for the best.  My machine has only one, so cant test a better
		// implementation.
		if((m_graphicsQueueIndex != -1) && (m_transferQueueIndex != -1) && (m_presentationQueueIndex != -1) &&
		   features.textureCompressionBC && features.fullDrawIndexUint32) {
			foundDevice    = true;
			selectedDevice = device;
			break;
		}
		m_graphicsQueueIndex     = -1;
		m_transferQueueIndex     = -1;
		m_presentationQueueIndex = -1;
	}
	CR_ASSERT(foundDevice, "Could not find a valid vulkan 1.2 graphics device");

	return selectedDevice;
}