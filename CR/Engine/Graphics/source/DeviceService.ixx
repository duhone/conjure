module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.DeviceService;

import CR.Engine.Core;
import CR.Engine.Platform;
import CR.Engine.Graphics.CommandPool;
import CR.Engine.Graphics.Commands;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Materials;
import CR.Engine.Graphics.Shaders;
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

		DeviceService(ceplat::Window& a_window, std::optional<glm::vec4> a_clearColor);
		~DeviceService()                    = default;
		DeviceService(const DeviceService&) = delete;
		DeviceService(DeviceService&&)      = delete;

		DeviceService& operator=(const DeviceService&) = delete;
		DeviceService& operator=(DeviceService&&)      = delete;

		void Update();

		void Stop();

	  private:
		void FindDevice();
		void BuildDevice();
		void CreateSwapChain();

		Context m_context;

		ceplat::Window& m_window;
		glm::ivec2 m_windowSize{0, 0};
		VkInstance m_instance;
		VkSurfaceKHR m_primarySurface;
		VkSwapchainKHR m_primarySwapChain;
		std::vector<VkImage> m_primarySwapChainImages;
		std::vector<VkImageView> m_primarySwapChainImageViews;
		VkRenderPass m_renderPass;    // only 1 currently, and only 1 subpass to go with it
		std::vector<VkFramebuffer> m_frameBuffers;
		VkSemaphore m_renderingFinished;    // need to block presenting until all rendering has completed
		VkFence m_frameFence;

		VkQueue m_graphicsQueue;
		VkQueue m_transferQueue;
		VkQueue m_presentationQueue;
		int32_t m_graphicsQueueIndex{-1};
		int32_t m_transferQueueIndex{-1};
		int32_t m_presentationQueueIndex{-1};

		// MSAA
		VkImage m_msaaImage;
		VkImageView m_msaaView;
		VmaAllocation m_msaaAlloc;

		uint32_t m_currentFrameBuffer{0};
		std::optional<glm::vec4> m_clearColor;

		CommandPool m_commandPool;
		cecore::Embedded<Shaders> m_shaders;
		cecore::Embedded<Materials> m_materials;
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

cegraph::DeviceService::DeviceService(ceplat::Window& a_window, std::optional<glm::vec4> a_clearColor) :
    m_window(a_window), m_clearColor(a_clearColor) {
	[[maybe_unused]] auto result = volkInitialize();
	CR_ASSERT(result == VK_SUCCESS, "Failed to initialize Volk");

	VkApplicationInfo appInfo;
	ClearStruct(appInfo);
	appInfo.pApplicationName   = CR_APP_NAME;
	appInfo.applicationVersion = CR_VERSION_APP;
	appInfo.pEngineName        = CR_ENGINE_NAME;
	appInfo.engineVersion      = CR_VERSION_ENGINE;
	appInfo.apiVersion         = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo;
	ClearStruct(createInfo);
	createInfo.pApplicationInfo    = &appInfo;
	createInfo.enabledLayerCount   = 0;
	createInfo.ppEnabledLayerNames = nullptr;
	createInfo.flags               = 0;

	std::vector<const char*> extensions;
	extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	createInfo.enabledExtensionCount   = (uint32_t)extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	result = vkCreateInstance(&createInfo, nullptr, &m_instance);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create vulkan instance");

	volkLoadInstance(m_instance);

	VkWin32SurfaceCreateInfoKHR win32Surface;
	ClearStruct(win32Surface);
	win32Surface.hinstance = reinterpret_cast<HINSTANCE>(m_window.GetHInstance());
	win32Surface.hwnd      = reinterpret_cast<HWND>(m_window.GetHWND());

	vkCreateWin32SurfaceKHR(m_instance, &win32Surface, nullptr, &m_primarySurface);

	FindDevice();
	BuildDevice();
	volkLoadDevice(m_context.Device);

	VmaAllocatorCreateInfo allocatorCreateInfo;
	memset(&allocatorCreateInfo, 0, sizeof(VmaAllocatorCreateInfo));
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
	allocatorCreateInfo.physicalDevice   = m_context.PhysicalDevice;
	allocatorCreateInfo.device           = m_context.Device;
	allocatorCreateInfo.instance         = m_instance;

	vmaCreateAllocator(&allocatorCreateInfo, &m_context.Allocator);

	CreateSwapChain();

	m_commandPool = CommandPool(m_context.Device, m_graphicsQueueIndex);
	m_shaders.emplace(m_context.Device);
	m_materials.emplace(m_context, *m_shaders, m_renderPass);
}

void cegraph::DeviceService::Stop() {
	vkDeviceWaitIdle(m_context.Device);

	m_materials.reset();
	m_shaders.reset();

	m_commandPool.ResetAll();
	m_commandPool = CommandPool();

	vkDestroyFence(m_context.Device, m_frameFence, nullptr);
	vkDestroySemaphore(m_context.Device, m_renderingFinished, nullptr);
	for(auto& framebuffer : m_frameBuffers) { vkDestroyFramebuffer(m_context.Device, framebuffer, nullptr); }

	vkDestroyRenderPass(m_context.Device, m_renderPass, nullptr);
	for(auto& imageView : m_primarySwapChainImageViews) {
		vkDestroyImageView(m_context.Device, imageView, nullptr);
	}
	m_primarySwapChainImageViews.clear();
	m_primarySwapChainImages.clear();

	vkDestroyImageView(m_context.Device, m_msaaView, nullptr);
	vmaDestroyImage(m_context.Allocator, m_msaaImage, m_msaaAlloc);

	vkDestroySwapchainKHR(m_context.Device, m_primarySwapChain, nullptr);

	vmaDestroyAllocator(m_context.Allocator);

	vkDestroyDevice(m_context.Device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_primarySurface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

void cegraph::DeviceService::Update() {
	vkAcquireNextImageKHR(m_context.Device, m_primarySwapChain, UINT64_MAX, VK_NULL_HANDLE, m_frameFence,
	                      &m_currentFrameBuffer);

	vkWaitForFences(m_context.Device, 1, &m_frameFence, VK_TRUE, UINT64_MAX);
	vkResetFences(m_context.Device, 1, &m_frameFence);

	m_commandPool.ResetAll();
	auto commandBuffer = m_commandPool.Begin();
	Commands::RenderPassBegin(commandBuffer, m_renderPass, m_frameBuffers[m_currentFrameBuffer], m_windowSize,
	                          m_clearColor);
	Commands::RenderPassEnd(commandBuffer);
	m_commandPool.End(commandBuffer);

	VkSubmitInfo subInfo;
	ClearStruct(subInfo);
	subInfo.commandBufferCount   = 0;
	subInfo.commandBufferCount   = 1;
	subInfo.pCommandBuffers      = &commandBuffer;
	subInfo.waitSemaphoreCount   = 0;
	subInfo.signalSemaphoreCount = 1;
	subInfo.pSignalSemaphores    = &m_renderingFinished;
	vkQueueSubmit(m_graphicsQueue, 1, &subInfo, VK_NULL_HANDLE);

	VkPresentInfoKHR presInfo;
	ClearStruct(presInfo);
	presInfo.waitSemaphoreCount = 1;
	presInfo.pWaitSemaphores    = &m_renderingFinished;
	presInfo.swapchainCount     = 1;
	presInfo.pSwapchains        = &m_primarySwapChain;
	presInfo.pImageIndices      = &m_currentFrameBuffer;
	vkQueuePresentKHR(m_presentationQueue, &presInfo);

	// Don't allow gpu to get behind, sacrifice performance for minimal latency.
	vkQueueWaitIdle(m_graphicsQueue);
	vkQueueWaitIdle(m_presentationQueue);
}

void cegraph::DeviceService::FindDevice() {
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
	features12.pNext = nullptr;

	VkPhysicalDeviceVulkan11Features features11;
	ClearStruct(features11);
	features11.pNext = &features12;

	VkPhysicalDeviceFeatures2 features;
	ClearStruct(features);
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

	m_context.PhysicalDevice = physicalDevices[foundDevice];
}

void cegraph::DeviceService::BuildDevice() {
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(m_context.PhysicalDevice, &memProps);
	for(uint32_t i = 0; i < memProps.memoryHeapCount; ++i) {
		auto heapSize   = memProps.memoryHeaps[i].size / 1024 / 1024;
		auto& heapFlags = memProps.memoryHeaps[i].flags;
		if(heapFlags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
			CR_LOG("Device Heap. Size {}MB", heapSize);
		} else {
			CR_LOG("Host Heap. Size {}MB", heapSize);
		}
	}

	for(uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
		auto& heapIndex = memProps.memoryTypes[i].heapIndex;
		auto& heapFlags = memProps.memoryTypes[i].propertyFlags;
		CR_LOG("\n");
		CR_LOG("Device Heap:  {} Type Index: {}", heapIndex, i);
		if(heapFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) { CR_LOG("  Device local"); }
		if(heapFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) { CR_LOG("  Host visible"); }
		if(heapFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) { CR_LOG("  Host cached"); }
		if(heapFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) { CR_LOG("  Host coherent"); }
		auto heapSize = (uint32_t)(memProps.memoryHeaps[heapIndex].size / 1024 / 1024);
		CR_LOG("  Heap Size: {}", heapSize);
	}

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
	createLogDevInfo.queueCreateInfoCount    = (int)size(queueInfos);
	createLogDevInfo.pQueueCreateInfos       = data(queueInfos);
	createLogDevInfo.pEnabledFeatures        = &requiredFeatures.features;
	createLogDevInfo.enabledLayerCount       = 0;
	createLogDevInfo.ppEnabledLayerNames     = nullptr;
	createLogDevInfo.enabledExtensionCount   = (uint32_t)size(deviceExtensions);
	createLogDevInfo.ppEnabledExtensionNames = data(deviceExtensions);

	if(vkCreateDevice(m_context.PhysicalDevice, &createLogDevInfo, nullptr, &m_context.Device) !=
	   VK_SUCCESS) {
		CR_ERROR("Failed to create a vulkan device");
	}

	vkGetDeviceQueue(m_context.Device, m_graphicsQueueIndex, graphicsQueueIndex, &m_graphicsQueue);
	vkGetDeviceQueue(m_context.Device, m_presentationQueueIndex, presentationQueueIndex,
	                 &m_presentationQueue);
	vkGetDeviceQueue(m_context.Device, m_transferQueueIndex, transferQueueIndex, &m_transferQueue);
}

void cegraph::DeviceService::CreateSwapChain() {
	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context.PhysicalDevice, m_primarySurface, &surfaceCaps);
	CR_LOG("current surface resolution: {}x{}", surfaceCaps.maxImageExtent.width,
	       surfaceCaps.maxImageExtent.height);
	CR_LOG("Min image count: {} Max image count: {}", surfaceCaps.minImageCount, surfaceCaps.maxImageCount);
	m_windowSize = glm::ivec2(surfaceCaps.maxImageExtent.width, surfaceCaps.maxImageExtent.height);

	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	uint32_t numFormats{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_context.PhysicalDevice, m_primarySurface, &numFormats, nullptr);
	surfaceFormats.resize(numFormats);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_context.PhysicalDevice, m_primarySurface, &numFormats,
	                                     surfaceFormats.data());

	CR_LOG("Supported surface formats:");
	for(const auto& format : surfaceFormats) {
		CR_LOG("    Format: {} ColorSpace {}", string_VkFormat(format.format),
		       string_VkColorSpaceKHR(format.colorSpace));
	}

	std::vector<VkPresentModeKHR> presentModes;
	uint32_t numPresModes{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_context.PhysicalDevice, m_primarySurface, &numPresModes,
	                                          nullptr);
	presentModes.resize(numPresModes);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_context.PhysicalDevice, m_primarySurface, &numPresModes,
	                                          presentModes.data());

	CR_LOG("Presentation modes:");
	for(const auto& mode : presentModes) {
		CR_LOG("    Presentation Mode: {}", string_VkPresentModeKHR(mode));
	}

	{
		// msaa image
		VkImageCreateInfo msaaCreateInfo;
		ClearStruct(msaaCreateInfo);
		msaaCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		msaaCreateInfo.extent.width  = surfaceCaps.maxImageExtent.width;
		msaaCreateInfo.extent.height = surfaceCaps.maxImageExtent.height;
		msaaCreateInfo.extent.depth  = 1;
		msaaCreateInfo.arrayLayers   = 1;
		msaaCreateInfo.mipLevels     = 1;
		msaaCreateInfo.samples       = VK_SAMPLE_COUNT_4_BIT;
		msaaCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
		msaaCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
		// TODO should be a transient attachment on mobile
		msaaCreateInfo.usage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		msaaCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		msaaCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
		msaaCreateInfo.flags         = 0;
		msaaCreateInfo.format        = VK_FORMAT_B8G8R8A8_SRGB;

		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage                   = VMA_MEMORY_USAGE_AUTO;
		allocCreateInfo.flags                   = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		allocCreateInfo.priority                = 1.0f;

		vmaCreateImage(m_context.Allocator, &msaaCreateInfo, &allocCreateInfo, &m_msaaImage, &m_msaaAlloc,
		               nullptr);

		VkImageViewCreateInfo viewInfo;
		ClearStruct(viewInfo);
		viewInfo.image                           = m_msaaImage;
		viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format                          = VK_FORMAT_B8G8R8A8_SRGB;
		viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel   = 0;
		viewInfo.subresourceRange.levelCount     = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount     = 1;

		vkCreateImageView(m_context.Device, &viewInfo, nullptr, &m_msaaView);
	}

	VkSwapchainCreateInfoKHR swapCreateInfo;
	ClearStruct(swapCreateInfo);
	swapCreateInfo.clipped         = true;
	swapCreateInfo.compositeAlpha  = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapCreateInfo.imageExtent     = surfaceCaps.maxImageExtent;
	swapCreateInfo.imageFormat     = VK_FORMAT_B8G8R8A8_SRGB;
	if(m_graphicsQueueIndex == m_presentationQueueIndex) {
		swapCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		swapCreateInfo.queueFamilyIndexCount = 0;
		swapCreateInfo.pQueueFamilyIndices   = nullptr;

	} else {
		swapCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		uint32_t queueFamilyIndices[] = {(uint32_t)m_graphicsQueueIndex, (uint32_t)m_presentationQueueIndex};
		swapCreateInfo.queueFamilyIndexCount = (uint32_t)std::size(queueFamilyIndices);
		swapCreateInfo.pQueueFamilyIndices   = std::data(queueFamilyIndices);
	}
	swapCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapCreateInfo.minImageCount    = 2;
	swapCreateInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
	swapCreateInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapCreateInfo.surface          = m_primarySurface;
	swapCreateInfo.imageArrayLayers = 1;

	vkCreateSwapchainKHR(m_context.Device, &swapCreateInfo, nullptr, &m_primarySwapChain);
	uint32_t numSwapChainImages{};
	vkGetSwapchainImagesKHR(m_context.Device, m_primarySwapChain, &numSwapChainImages, nullptr);
	m_primarySwapChainImages.resize(numSwapChainImages);
	vkGetSwapchainImagesKHR(m_context.Device, m_primarySwapChain, &numSwapChainImages,
	                        m_primarySwapChainImages.data());

	for(const auto& image : m_primarySwapChainImages) {
		VkImageViewCreateInfo viewInfo;
		ClearStruct(viewInfo);
		viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.format                          = VK_FORMAT_B8G8R8A8_SRGB;
		viewInfo.image                           = image;
		viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount     = 1;
		viewInfo.subresourceRange.baseMipLevel   = 0;
		viewInfo.subresourceRange.levelCount     = 1;
		m_primarySwapChainImageViews.emplace_back();
		vkCreateImageView(m_context.Device, &viewInfo, nullptr, &m_primarySwapChainImageViews.back());
	}

	VkAttachmentDescription attatchDescs[2];
	ClearStruct(attatchDescs[0]);
	ClearStruct(attatchDescs[1]);
	attatchDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attatchDescs[0].finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attatchDescs[0].format        = VK_FORMAT_B8G8R8A8_SRGB;
	if(m_clearColor.has_value()) {
		attatchDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	} else {
		attatchDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
	// TODO should be dont care for mobile, would be transient and never need to be stored to memory
	attatchDescs[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attatchDescs[0].samples        = VK_SAMPLE_COUNT_4_BIT;
	attatchDescs[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attatchDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attatchDescs[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	attatchDescs[1].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attatchDescs[1].format         = VK_FORMAT_B8G8R8A8_SRGB;
	attatchDescs[1].loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	// TODO should be dont care for mobile
	attatchDescs[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attatchDescs[1].samples        = VK_SAMPLE_COUNT_1_BIT;
	attatchDescs[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attatchDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkAttachmentReference attachRefs[2];
	ClearStruct(attachRefs[0]);
	ClearStruct(attachRefs[1]);
	attachRefs[0].attachment = 0;
	attachRefs[0].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachRefs[1].attachment = 1;
	attachRefs[1].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDesc;
	ClearStruct(subpassDesc);
	subpassDesc.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments    = &attachRefs[0];
	subpassDesc.pResolveAttachments  = &attachRefs[1];

	VkRenderPassCreateInfo renderPassInfo;
	ClearStruct(renderPassInfo);
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments    = attatchDescs;
	renderPassInfo.subpassCount    = 1;
	renderPassInfo.pSubpasses      = &subpassDesc;

	vkCreateRenderPass(m_context.Device, &renderPassInfo, nullptr, &m_renderPass);

	for(auto& imageView : m_primarySwapChainImageViews) {
		VkFramebufferCreateInfo framebufferInfo;
		ClearStruct(framebufferInfo);
		const VkImageView attachments[] = {m_msaaView, imageView};
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments    = attachments;
		framebufferInfo.width           = m_windowSize.x;
		framebufferInfo.height          = m_windowSize.y;
		framebufferInfo.renderPass      = m_renderPass;
		framebufferInfo.layers          = 1;

		m_frameBuffers.emplace_back();
		vkCreateFramebuffer(m_context.Device, &framebufferInfo, nullptr, &m_frameBuffers.back());
	}

	VkSemaphoreCreateInfo semInfo;
	ClearStruct(semInfo);
	vkCreateSemaphore(m_context.Device, &semInfo, nullptr, &m_renderingFinished);

	VkFenceCreateInfo fenceInfo;
	ClearStruct(fenceInfo);
	vkCreateFence(m_context.Device, &fenceInfo, nullptr, &m_frameFence);
}