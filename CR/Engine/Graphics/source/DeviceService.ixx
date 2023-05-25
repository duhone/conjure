module;

#include "core/Log.h"

#include "volk.h"

export module CR.Engine.Graphics.DeviceService;

import CR.Engine.Core;
import CR.Engine.Platform;
import CR.Engine.Graphics.Utils;

import <algorithm>;
import <optional>;
import <ranges>;
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
		VkPhysicalDevice FindDevice();
		void BuildDevice(VkPhysicalDevice& selectedDevice);

		ceplat::Window& m_window;
		VkInstance m_instance;
		VkSurfaceKHR m_primarySurface;

		VkDevice m_device;
		int32_t m_graphicsQueueIndex{-1};
		int32_t m_transferQueueIndex{-1};
		int32_t m_presentationQueueIndex{-1};
		VkQueue m_graphicsQueue;
		VkQueue m_transferQueue;
		VkQueue m_presentationQueue;
		int32_t m_deviceMemoryIndex{-1};
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

cegraph::DeviceService::DeviceService(ceplat::Window& a_window) : m_window(a_window) {
	[[maybe_unused]] auto result = volkInitialize();
	CR_ASSERT(result == VK_SUCCESS, "Failed to initialize Volk");

	VkApplicationInfo appInfo;
	ClearStruct(appInfo);
	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext              = nullptr;
	appInfo.pApplicationName   = CR_APP_NAME;
	appInfo.applicationVersion = CR_VERSION_APP;
	appInfo.pEngineName        = CR_ENGINE_NAME;
	appInfo.engineVersion      = CR_VERSION_ENGINE;
	appInfo.apiVersion         = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo;
	ClearStruct(createInfo);
	createInfo.sType               = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext               = nullptr;
	createInfo.pApplicationInfo    = &appInfo;
	createInfo.enabledLayerCount   = 0;
	createInfo.ppEnabledLayerNames = nullptr;
	createInfo.flags               = 0;

	std::vector<const char*> extensions;
	extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	createInfo.enabledExtensionCount   = extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	result = vkCreateInstance(&createInfo, nullptr, &m_instance);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create vulkan instance");

	volkLoadInstance(m_instance);

	VkWin32SurfaceCreateInfoKHR win32Surface;
	ClearStruct(win32Surface);
	win32Surface.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32Surface.hinstance = reinterpret_cast<HINSTANCE>(m_window.GetHInstance());
	win32Surface.hwnd      = reinterpret_cast<HWND>(m_window.GetHWND());

	vkCreateWin32SurfaceKHR(m_instance, &win32Surface, nullptr, &m_primarySurface);

	VkPhysicalDevice selectedDevice = FindDevice();
	BuildDevice(selectedDevice);
}

void cegraph::DeviceService::Stop() {
	vkDestroyDevice(m_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_primarySurface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

void cegraph::DeviceService::Update() {}

VkPhysicalDevice cegraph::DeviceService::FindDevice() {
	std::vector<VkPhysicalDevice> physicalDevices;
	uint32_t deviceCount{};
	[[maybe_unused]] auto result = vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
	CR_ASSERT(result == VK_SUCCESS, "Failed to enumerate vulkan devices");
	physicalDevices.resize(deviceCount);
	result = vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());
	CR_ASSERT(result == VK_SUCCESS, "Failed to enumerate vulkan devices");

	CR_ASSERT(!physicalDevices.empty(), "No GPU's that support Vulkan on this machine");

	int32_t foundDevice = -1;
	std::vector<VkPhysicalDeviceProperties> physicalDevicesProps;
	physicalDevicesProps.resize(physicalDevices.size());
	for(int32_t device = 0; device < physicalDevices.size(); ++device) {
		vkGetPhysicalDeviceProperties(physicalDevices[device], &physicalDevicesProps[device]);
	}

	// First try to find a discrete gpu.
	for(int32_t prop = 0; prop < physicalDevicesProps.size(); ++prop) {
		if(physicalDevicesProps[prop].deviceType ==
		   VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			foundDevice = prop;
			break;
		}
	}

	// Next find an integrated GPU
	if(foundDevice == -1) {
		for(int32_t prop = 0; prop < physicalDevicesProps.size(); ++prop) {
			if(physicalDevicesProps[prop].deviceType ==
			   VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
				foundDevice = prop;
				break;
			}
		}
	}

	// Not going to support any other types of GPU's.
	CR_ASSERT(foundDevice != -1, "Could not find a suitable GPU");

	VkPhysicalDeviceProperties& props = physicalDevicesProps[foundDevice];
	CR_LOG("Device Name : {}", props.deviceName);
	CR_ASSERT(props.apiVersion >= VK_API_VERSION_1_2,
	          "Require Vulkan 1.2 or greater, check for newer drivers");

	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(physicalDevices[foundDevice], &memProps);
	CR_LOG("  Device max allocations: {}", props.limits.maxMemoryAllocationCount);
	CR_LOG("  Device max array layers: {}", props.limits.maxImageArrayLayers);
	CR_LOG("  Device max 2D image dimensions: {}", props.limits.maxImageDimension2D);
	CR_LOG("  Device max multisample: {}", (VkFlags)props.limits.framebufferColorSampleCounts);
	for(uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
		if(memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
			CR_LOG("  Device Local Memory Amount: {}MB",
			       memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size / (1024 * 1024));
		}
	}

	std::vector<VkQueueFamilyProperties> queueProps;
	uint32_t numQueueProps{};
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[foundDevice], &numQueueProps, nullptr);
	queueProps.resize(numQueueProps);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[foundDevice], &numQueueProps, queueProps.data());

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
		// for now, assuming there will be a queue that supports graphics and compute.
		if((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
		   (queueProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && queueProps[i].queueCount >= 1) {
			supportsGraphics = true;
			supportsCompute  = true;
			graphicsQueues.push_back(i);
			CR_LOG("  supports graphics and compute");
		}
		if((queueProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && queueProps[i].queueCount >= 1) {
			supportsTransfer = true;
			transferQueues.push_back(i);
			CR_LOG("  supports transfer");
		}

		VkBool32 surfaceSupported{};
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[foundDevice], i, m_primarySurface,
		                                     &surfaceSupported);
		if(surfaceSupported) {
			supportsPresentation = true;
			presentationQueues.push_back(i);
			CR_LOG("  supports presentation");
		}

		// for transfers, prefer a dedicated transfer queue(. If more than one,
		// grab first
		if(!supportsGraphics && !supportsCompute && !supportsPresentation && supportsTransfer &&
		   !dedicatedTransfer.has_value()) {
			dedicatedTransfer = i;
		}
		// For graphics, we prefer one that does graphics, compute, and presentation
		if(supportsGraphics && supportsCompute && supportsPresentation &&
		   !graphicsAndPresentation.has_value()) {
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

	VkPhysicalDeviceVulkan12Features features12;
	ClearStruct(features12);
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features12.pNext = nullptr;

	VkPhysicalDeviceVulkan11Features features11;
	ClearStruct(features11);
	features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	features11.pNext = &features12;

	VkPhysicalDeviceFeatures2 features;
	ClearStruct(features);
	features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features.pNext = &features11;
	vkGetPhysicalDeviceFeatures2(physicalDevices[foundDevice], &features);
	CR_ASSERT(features.features.textureCompressionBC, "Require support for BC texture compression");
	CR_ASSERT(features.features.multiDrawIndirect, "Require support for multi draw indirect");
	CR_ASSERT(features11.uniformAndStorageBuffer16BitAccess, "Require support for 16 bit types in buffers");
	CR_ASSERT(features11.storagePushConstant16, "Require support for 16 bit types in push constants");
	CR_ASSERT(features12.drawIndirectCount, "Require support for multi draw indirect");
	CR_ASSERT(features12.descriptorIndexing, "Require support for descriptor indexing");
	CR_ASSERT(features12.shaderUniformTexelBufferArrayDynamicIndexing,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.shaderStorageTexelBufferArrayDynamicIndexing,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.shaderUniformBufferArrayNonUniformIndexing,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.shaderSampledImageArrayNonUniformIndexing,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.shaderStorageBufferArrayNonUniformIndexing,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.shaderStorageImageArrayNonUniformIndexing,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.shaderUniformTexelBufferArrayNonUniformIndexing,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.descriptorBindingSampledImageUpdateAfterBind,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.descriptorBindingStorageImageUpdateAfterBind,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.descriptorBindingStorageBufferUpdateAfterBind,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.descriptorBindingUniformTexelBufferUpdateAfterBind,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.descriptorBindingStorageTexelBufferUpdateAfterBind,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.descriptorBindingUpdateUnusedWhilePending,
	          "Require support for descriptor indexing");
	CR_ASSERT(features12.descriptorBindingPartiallyBound, "Require support for descriptor indexing");
	CR_ASSERT(features12.uniformBufferStandardLayout, "Require support for standard layout");

	CR_ASSERT(foundDevice != -1, "Could not find a valid vulkan 1.2 graphics device");

	return physicalDevices[foundDevice];
}

void cegraph::DeviceService::BuildDevice(VkPhysicalDevice& selectedDevice) {
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(selectedDevice, &memProps);
	for(uint32_t i = 0; i < memProps.memoryHeapCount; ++i) {
		auto heapSize   = memProps.memoryHeaps[i].size / 1024 / 1024;
		auto& heapFlags = memProps.memoryHeaps[i].flags;
		if(heapFlags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
			CR_LOG("Device Heap. Size {}MB", heapSize);
		} else {
			CR_LOG("Host Heap. Size {}MB", heapSize);
		}
	}
	uint32_t heapSize{};
	for(uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
		auto& heapIndex = memProps.memoryTypes[i].heapIndex;
		auto& heapFlags = memProps.memoryTypes[i].propertyFlags;
		CR_LOG("\n");
		CR_LOG("Device Heap:  {} Type Index: {}", heapIndex, i);
		if(heapFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) { CR_LOG("  Device local"); }
		if(heapFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) { CR_LOG("  Host visible"); }
		if(heapFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) { CR_LOG("  Host cached"); }
		if(heapFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) { CR_LOG("  Host coherent"); }

		// Only going to support unified memory and rebar. This gives the fastest loading, but won't work on
		// older GPU's
		if((heapFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0 and
		   (heapFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 and
		   (heapFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0 and
		   (heapFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == 0 and (m_deviceMemoryIndex == -1)) {
			m_deviceMemoryIndex = i;
			heapSize            = memProps.memoryHeaps[heapIndex].size / 1024 / 1024;
		}
	}
	CR_LOG("Chosen device Heap:  {} Size {}MB", m_deviceMemoryIndex, heapSize);

	VkPhysicalDeviceFeatures2 requiredFeatures;
	ClearStruct(requiredFeatures);
	requiredFeatures.sType                         = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	requiredFeatures.features.sampleRateShading    = true;
	requiredFeatures.features.textureCompressionBC = true;
	requiredFeatures.features.fullDrawIndexUint32  = true;
	requiredFeatures.features.shaderSampledImageArrayDynamicIndexing = true;

	VkPhysicalDeviceVulkan12Features requiredFeatures12;
	ClearStruct(requiredFeatures12);
	requiredFeatures12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	requiredFeatures12.descriptorBindingPartiallyBound              = true;
	requiredFeatures12.descriptorBindingVariableDescriptorCount     = true;
	requiredFeatures12.descriptorIndexing                           = true;
	requiredFeatures12.shaderSampledImageArrayNonUniformIndexing    = true;
	requiredFeatures12.shaderInputAttachmentArrayDynamicIndexing    = true;
	requiredFeatures12.runtimeDescriptorArray                       = true;
	requiredFeatures12.descriptorBindingSampledImageUpdateAfterBind = true;
	requiredFeatures.pNext                                          = &requiredFeatures12;

	int32_t graphicsQueueIndex     = 0;
	int32_t presentationQueueIndex = 0;
	int32_t transferQueueIndex     = 0;
	int32_t queueIndexMap[256];    // surely no more than 256 queue families for all time
	std::ranges::fill(queueIndexMap, -1);

	float graphicsPriority = 1.0f;
	float transferPriority = 0.0f;
	VkDeviceQueueCreateInfo queueInfo;
	ClearStruct(queueInfo);
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = nullptr;
	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	queueInfos.reserve(3);
	queueInfo.queueFamilyIndex = m_graphicsQueueIndex;
	queueInfo.queueCount       = 1;
	queueInfo.pQueuePriorities = &graphicsPriority;
	queueInfos.push_back(queueInfo);
	++queueIndexMap[m_graphicsQueueIndex];
	graphicsQueueIndex = queueIndexMap[m_graphicsQueueIndex];
	if(m_graphicsQueueIndex != m_presentationQueueIndex) {
		queueInfo.queueFamilyIndex = m_presentationQueueIndex;
		queueInfos.push_back(queueInfo);
		++queueIndexMap[m_presentationQueueIndex];
	}
	presentationQueueIndex     = queueIndexMap[m_presentationQueueIndex];
	queueInfo.queueFamilyIndex = m_transferQueueIndex;
	queueInfo.pQueuePriorities = &transferPriority;
	queueInfos.push_back(queueInfo);
	++queueIndexMap[m_transferQueueIndex];
	transferQueueIndex = queueIndexMap[m_transferQueueIndex];

	std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	VkDeviceCreateInfo createLogDevInfo;
	ClearStruct(createLogDevInfo);
	createLogDevInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createLogDevInfo.pNext                   = nullptr;
	createLogDevInfo.queueCreateInfoCount    = (int)size(queueInfos);
	createLogDevInfo.pQueueCreateInfos       = data(queueInfos);
	createLogDevInfo.pEnabledFeatures        = &requiredFeatures.features;
	createLogDevInfo.enabledLayerCount       = 0;
	createLogDevInfo.ppEnabledLayerNames     = nullptr;
	createLogDevInfo.enabledExtensionCount   = (uint32_t)size(deviceExtensions);
	createLogDevInfo.ppEnabledExtensionNames = data(deviceExtensions);

	if(vkCreateDevice(selectedDevice, &createLogDevInfo, nullptr, &m_device) != VK_SUCCESS) {
		CR_ERROR("Failed to create a vulkan device");
	}
}