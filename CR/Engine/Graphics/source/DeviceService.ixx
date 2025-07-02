module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.DeviceService;

import CR.Engine.Core;
import CR.Engine.Platform;
import CR.Engine.Graphics.CommandPool;
import CR.Engine.Graphics.Commands;
import CR.Engine.Graphics.ComputePipelines;
import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.GraphicsThread;
import CR.Engine.Graphics.Materials;
import CR.Engine.Graphics.Shaders;
import CR.Engine.Graphics.SpritesInternal;
import CR.Engine.Graphics.Textures;
import CR.Engine.Graphics.UniformBuffer;
import CR.Engine.Graphics.Utils;
import CR.Engine.Graphics.VertexBuffers;

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

		bool Update();
		bool ReInitialize();

		void Stop();

	  private:
		void FindDevice(Context& context);
		void BuildDevice(Context& context);
		bool CreateSwapChain(const Context& context);
		void DestroySwapChain();

		ceplat::Window& m_window;
		glm::ivec2 m_windowSize{0, 0};
		VkInstance m_instance{};
		VkSurfaceKHR m_primarySurface{};
		VkSwapchainKHR m_primarySwapChain{};
		std::vector<VkImage> m_primarySwapChainImages{};
		std::vector<VkImageView> m_primarySwapChainImageViews{};
		VkRenderPass m_renderPass{};    // only 1 currently, and only 1 subpass to go with it
		std::vector<VkFramebuffer> m_frameBuffers;
		VkSemaphore m_renderingFinished;    // need to block presenting until all rendering has completed
		VkFence m_frameFence;
		VkDescriptorPool m_globalDescriptorPool{};

		VkQueue m_graphicsQueue;
		VkQueue m_transferQueue;
		VkQueue m_presentationQueue;
		int32_t m_presentationQueueIndex{-1};

		// MSAA
		VkImage m_msaaImage{};
		VkImageView m_msaaView{};
		VmaAllocation m_msaaAlloc{};

		uint32_t m_currentFrameBuffer{0};
		std::optional<glm::vec4> m_clearColor;

		CommandPool m_commandPool;
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

namespace {
#pragma pack(push)
#pragma pack(1)
	struct GlobalUniformBuffer {
		glm::vec2 InvScreenSize;
	};
#pragma pack(pop)
}    // namespace

cegraph::DeviceService::DeviceService(ceplat::Window& a_window, std::optional<glm::vec4> a_clearColor) :
    m_window(a_window), m_clearColor(a_clearColor) {
	Context context;

	[[maybe_unused]] auto result = volkInitialize();
	CR_ASSERT(result == VK_SUCCESS, "Failed to initialize Volk");

	VkApplicationInfo appInfo;
	ClearStruct(appInfo);
	appInfo.pApplicationName   = CR_APP_NAME;
	appInfo.applicationVersion = CR_VERSION_APP;
	appInfo.pEngineName        = CR_ENGINE_NAME;
	appInfo.engineVersion      = CR_VERSION_ENGINE;
	appInfo.apiVersion         = VK_API_VERSION_1_4;

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

	FindDevice(context);
	BuildDevice(context);
	volkLoadDevice(context.Device);

	VmaAllocatorCreateInfo allocatorCreateInfo;
	memset(&allocatorCreateInfo, 0, sizeof(VmaAllocatorCreateInfo));
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
	allocatorCreateInfo.physicalDevice   = context.PhysicalDevice;
	allocatorCreateInfo.device           = context.Device;
	allocatorCreateInfo.instance         = m_instance;

	vmaCreateAllocator(&allocatorCreateInfo, &context.Allocator);

	bool swapChainCreated = CreateSwapChain(context);
	CR_ASSERT(swapChainCreated, "Unable to create initial swap chain");

	VkSemaphoreCreateInfo semInfo;
	ClearStruct(semInfo);
	vkCreateSemaphore(context.Device, &semInfo, nullptr, &m_renderingFinished);

	VkFenceCreateInfo fenceInfo;
	ClearStruct(fenceInfo);
	vkCreateFence(context.Device, &fenceInfo, nullptr, &m_frameFence);

	context.DisplayTicksPerFrame = Constants::c_designRefreshRate / a_window.GetRefreshRate();

	// set up global texture descriptor. only 1 frame in flight, so easy.
	VkDescriptorPoolSize poolSizes[2];
	poolSizes[0].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = cegraph::Constants::c_maxTextures;
	poolSizes[1].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	ClearStruct(poolInfo);
	poolInfo.poolSizeCount = std::size(poolSizes);
	poolInfo.pPoolSizes    = std::data(poolSizes);
	poolInfo.maxSets       = 1;

	result = vkCreateDescriptorPool(context.Device, &poolInfo, nullptr, &m_globalDescriptorPool);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create descriptor pool");

	VkDescriptorSetLayoutBinding dslBinding[2];
	ClearStruct(dslBinding[0]);
	ClearStruct(dslBinding[1]);
	dslBinding[0].binding         = 0;
	dslBinding[0].descriptorCount = Constants::c_maxTextures;
	dslBinding[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	dslBinding[0].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
	dslBinding[1].binding         = 1;
	dslBinding[1].descriptorCount = 1;
	dslBinding[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	dslBinding[1].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo dslInfo;
	ClearStruct(dslInfo);
	dslInfo.bindingCount = (uint32_t)std::size(dslBinding);
	dslInfo.pBindings    = dslBinding;
	dslInfo.flags        = 0;

	result =
	    vkCreateDescriptorSetLayout(context.Device, &dslInfo, nullptr, &context.GlobalDescriptorSetLayout);
	CR_ASSERT(result == VK_SUCCESS, "failed to create a descriptor set layout");

	VkDescriptorSetAllocateInfo info{};
	ClearStruct(info);
	info.descriptorPool     = m_globalDescriptorPool;
	info.descriptorSetCount = 1;
	info.pSetLayouts        = &context.GlobalDescriptorSetLayout;

	result = vkAllocateDescriptorSets(context.Device, &info, &context.GlobalDescriptorSet);
	CR_ASSERT(result == VK_SUCCESS, "Failed to create descriptor set");

	SetContext(context);

	m_commandPool = CommandPool(context.GraphicsQueueIndex);
	GraphicsThread::Initialize(m_transferQueue);
	Shaders::Initialize();
	Textures::Initialize();
	Materials::Initialize(m_renderPass);
	ComputePipelines::Initialize();
	UniformBuffer::Initialize();
	VertexBuffers::Initialize();
	Sprites::Initialize();

	ComputePipelines::FinishInitialize();
	Materials::FinishInitialize();
}

void cegraph::DeviceService::Stop() {
	const Context& context = GetContext();

	vkDeviceWaitIdle(context.Device);

	vkDestroyDescriptorPool(context.Device, m_globalDescriptorPool, nullptr);
	m_globalDescriptorPool = nullptr;

	vkDestroyDescriptorSetLayout(context.Device, context.GlobalDescriptorSetLayout, nullptr);

	Sprites::Shutdown();
	VertexBuffers::Shutdown();
	UniformBuffer::Shutdown();
	Textures::Shutdown();
	ComputePipelines::Shutdown();
	Materials::Shutdown();
	Shaders::Shutdown();
	GraphicsThread::Shutdown();

	m_commandPool.ResetAll();
	m_commandPool = CommandPool();

	vkDestroyFence(context.Device, m_frameFence, nullptr);
	vkDestroySemaphore(context.Device, m_renderingFinished, nullptr);

	DestroySwapChain();

	vmaDestroyAllocator(context.Allocator);

	vkDestroyDevice(context.Device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_primarySurface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

bool cegraph::DeviceService::Update() {
	const Context& context = GetContext();

	if(m_primarySwapChain == nullptr) { return false; }

	vkDeviceWaitIdle(GetContext().Device);
	VkResult result = vkAcquireNextImageKHR(context.Device, m_primarySwapChain, UINT64_MAX, VK_NULL_HANDLE,
	                                        m_frameFence, &m_currentFrameBuffer);
	if(result == VK_ERROR_OUT_OF_DATE_KHR) { return false; }

	vkWaitForFences(context.Device, 1, &m_frameFence, VK_TRUE, UINT64_MAX);
	vkResetFences(context.Device, 1, &m_frameFence);

	if(result == VK_SUBOPTIMAL_KHR) { return false; }

	vkDeviceWaitIdle(GetContext().Device);
	m_commandPool.ResetAll();
	auto commandBuffer = m_commandPool.Begin();

	Textures::Update(commandBuffer);
	UniformBuffer::Update();

	auto uboMap              = UniformBuffer::Map(sizeof(GlobalUniformBuffer));
	GlobalUniformBuffer* ubo = (GlobalUniformBuffer*)uboMap.Data;
	ubo->InvScreenSize.x     = 1.0f / m_windowSize.x;
	ubo->InvScreenSize.y     = 1.0f / m_windowSize.y;

	VkDescriptorBufferInfo bufferInfo;
	ClearStruct(bufferInfo);
	bufferInfo.buffer = uboMap.Buffer;
	bufferInfo.offset = 0;
	bufferInfo.range  = uboMap.Size;

	VkWriteDescriptorSet writeSet;
	ClearStruct(writeSet);
	writeSet.dstBinding      = 1;
	writeSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeSet.descriptorCount = 1;
	writeSet.pBufferInfo     = &bufferInfo;
	writeSet.dstSet          = context.GlobalDescriptorSet;
	vkUpdateDescriptorSets(context.Device, 1, &writeSet, 0, nullptr);

	Sprites::Update();

	VkViewport viewport;
	viewport.x        = 0.0f;
	viewport.y        = 0.0f;
	viewport.width    = (float)m_windowSize.x;
	viewport.height   = (float)m_windowSize.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent.width  = m_windowSize.x;
	scissor.extent.height = m_windowSize.y;

	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	Commands::RenderPassBegin(commandBuffer, m_renderPass, m_frameBuffers[m_currentFrameBuffer], m_windowSize,
	                          m_clearColor);

	Sprites::Render(commandBuffer);

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
	result                      = vkQueuePresentKHR(m_presentationQueue, &presInfo);
	if(result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR) { return false; }

	// Don't allow gpu to get behind, sacrifice performance for minimal latency.
	vkQueueWaitIdle(m_graphicsQueue);
	vkQueueWaitIdle(m_presentationQueue);

	return true;
}

void cegraph::DeviceService::FindDevice(Context& context) {
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
		context.TransferQueueIndex = dedicatedTransfer.value();
	} else if(!transferQueues.empty()) {
		context.TransferQueueIndex = transferQueues[0];
	} else {
		CR_LOG("Could not find a valid vulkan transfer queue");
	}

	if(graphicsAndPresentation.has_value()) {
		context.GraphicsQueueIndex = m_presentationQueueIndex = graphicsAndPresentation.value();
	} else if(!graphicsQueues.empty() && !presentationQueues.empty()) {
		context.GraphicsQueueIndex = graphicsQueues[0];
		m_presentationQueueIndex   = presentationQueues[0];
	} else {
		if(graphicsQueues.empty()) { CR_LOG("Could not find a valid vulkan graphics queue"); }
		if(presentationQueues.empty()) { CR_LOG("Could not find a valid presentation queue"); }
	}

	CR_LOG("graphics queue family: {} transfer queue index: {} presentation queue index: {}",
	       context.GraphicsQueueIndex, context.TransferQueueIndex, m_presentationQueueIndex);

	VkPhysicalDeviceRobustness2FeaturesKHR featuresRobust;
	ClearStruct(featuresRobust);
	featuresRobust.pNext = nullptr;

	VkPhysicalDeviceVulkan14Features features14;
	ClearStruct(features14);
	features14.pNext = &featuresRobust;

	VkPhysicalDeviceVulkan13Features features13;
	ClearStruct(features13);
	features13.pNext = &features14;

	VkPhysicalDeviceVulkan12Features features12;
	ClearStruct(features12);
	features12.pNext = &features13;

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

	context.PhysicalDevice = physicalDevices[foundDevice];
}

void cegraph::DeviceService::BuildDevice(Context& context) {
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(context.PhysicalDevice, &memProps);
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

	VkPhysicalDeviceVulkan11Features requiredFeatures11;
	ClearStruct(requiredFeatures11);
	requiredFeatures.pNext = &requiredFeatures11;

	VkPhysicalDeviceVulkan12Features requiredFeatures12;
	ClearStruct(requiredFeatures12);
	requiredFeatures12.descriptorBindingPartiallyBound              = true;
	requiredFeatures12.descriptorBindingVariableDescriptorCount     = true;
	requiredFeatures12.descriptorIndexing                           = true;
	requiredFeatures12.shaderSampledImageArrayNonUniformIndexing    = true;
	requiredFeatures12.shaderInputAttachmentArrayDynamicIndexing    = true;
	requiredFeatures12.runtimeDescriptorArray                       = true;
	requiredFeatures12.descriptorBindingSampledImageUpdateAfterBind = true;
	requiredFeatures11.pNext                                        = &requiredFeatures12;

	VkPhysicalDeviceVulkan13Features requiredFeatures13;
	ClearStruct(requiredFeatures13);
	requiredFeatures12.pNext = &requiredFeatures13;

	VkPhysicalDeviceVulkan14Features requiredFeatures14;
	ClearStruct(requiredFeatures14);
	requiredFeatures14.pushDescriptor = true;
	requiredFeatures13.pNext          = &requiredFeatures14;

	VkPhysicalDeviceRobustness2FeaturesKHR requiredFeaturesRobust;
	ClearStruct(requiredFeaturesRobust);
	requiredFeaturesRobust.nullDescriptor = VK_TRUE;
	// requiredFeatures14.pNext              = &requiredFeaturesRobust;

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
	queueInfo.queueFamilyIndex = context.GraphicsQueueIndex;
	queueInfo.queueCount       = 1;
	queueInfo.pQueuePriorities = &graphicsPriority;
	queueInfos.push_back(queueInfo);
	++queueIndexMap[context.GraphicsQueueIndex];
	graphicsQueueIndex = queueIndexMap[context.GraphicsQueueIndex];
	if(context.GraphicsQueueIndex != m_presentationQueueIndex) {
		queueInfo.queueFamilyIndex = m_presentationQueueIndex;
		queueInfos.push_back(queueInfo);
		++queueIndexMap[m_presentationQueueIndex];
	}
	presentationQueueIndex = queueIndexMap[m_presentationQueueIndex];
	if(context.GraphicsQueueIndex != context.TransferQueueIndex) {
		queueInfo.queueFamilyIndex = context.TransferQueueIndex;
		queueInfo.pQueuePriorities = &transferPriority;
		queueInfos.push_back(queueInfo);
		++queueIndexMap[context.TransferQueueIndex];
	}
	transferQueueIndex = queueIndexMap[context.TransferQueueIndex];

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
	createLogDevInfo.pNext                   = &requiredFeaturesRobust;

	if(vkCreateDevice(context.PhysicalDevice, &createLogDevInfo, nullptr, &context.Device) != VK_SUCCESS) {
		CR_ERROR("Failed to create a vulkan device");
	}

	vkGetDeviceQueue(context.Device, context.GraphicsQueueIndex, graphicsQueueIndex, &m_graphicsQueue);
	vkGetDeviceQueue(context.Device, m_presentationQueueIndex, presentationQueueIndex, &m_presentationQueue);
	vkGetDeviceQueue(context.Device, context.TransferQueueIndex, transferQueueIndex, &m_transferQueue);
}

bool cegraph::DeviceService::CreateSwapChain(const Context& context) {
	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context.PhysicalDevice, m_primarySurface, &surfaceCaps);
	CR_LOG("current surface resolution: {}x{}", surfaceCaps.maxImageExtent.width,
	       surfaceCaps.maxImageExtent.height);
	CR_LOG("Min image count: {} Max image count: {}", surfaceCaps.minImageCount, surfaceCaps.maxImageCount);
	m_windowSize = glm::ivec2(surfaceCaps.maxImageExtent.width, surfaceCaps.maxImageExtent.height);

	if(m_windowSize.x == 0 || m_windowSize.y == 0) {
		CR_LOG("Could not recreate swap chain, 0 size window");
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	uint32_t numFormats{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(context.PhysicalDevice, m_primarySurface, &numFormats, nullptr);
	surfaceFormats.resize(numFormats);
	vkGetPhysicalDeviceSurfaceFormatsKHR(context.PhysicalDevice, m_primarySurface, &numFormats,
	                                     surfaceFormats.data());

	CR_LOG("Supported surface formats:");
	for(const auto& format : surfaceFormats) {
		CR_LOG("    Format: {} ColorSpace {}", string_VkFormat(format.format),
		       string_VkColorSpaceKHR(format.colorSpace));
	}

	std::vector<VkPresentModeKHR> presentModes;
	uint32_t numPresModes{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(context.PhysicalDevice, m_primarySurface, &numPresModes,
	                                          nullptr);
	presentModes.resize(numPresModes);
	vkGetPhysicalDeviceSurfacePresentModesKHR(context.PhysicalDevice, m_primarySurface, &numPresModes,
	                                          presentModes.data());

	CR_LOG("Presentation modes:");
	for(const auto& mode : presentModes) {
		CR_LOG("    Presentation Mode: {}", string_VkPresentModeKHR(mode));
	}

	VkResult vkResult{};

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

		vkResult = vmaCreateImage(context.Allocator, &msaaCreateInfo, &allocCreateInfo, &m_msaaImage,
		                          &m_msaaAlloc, nullptr);
		if(vkResult != VK_SUCCESS) {
			CR_LOG("Failed to create msaa image");
			DestroySwapChain();
			return false;
		}

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

		vkResult = vkCreateImageView(context.Device, &viewInfo, nullptr, &m_msaaView);
		if(vkResult != VK_SUCCESS) {
			CR_LOG("Failed to create msaa image view");
			DestroySwapChain();
			return false;
		}
	}

	VkSwapchainCreateInfoKHR swapCreateInfo;
	ClearStruct(swapCreateInfo);
	swapCreateInfo.clipped         = true;
	swapCreateInfo.compositeAlpha  = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapCreateInfo.imageExtent     = surfaceCaps.maxImageExtent;
	swapCreateInfo.imageFormat     = VK_FORMAT_B8G8R8A8_SRGB;
	if(context.GraphicsQueueIndex == m_presentationQueueIndex) {
		swapCreateInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		swapCreateInfo.queueFamilyIndexCount = 0;
		swapCreateInfo.pQueueFamilyIndices   = nullptr;

	} else {
		swapCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		uint32_t queueFamilyIndices[]        = {(uint32_t)context.GraphicsQueueIndex,
		                                        (uint32_t)m_presentationQueueIndex};
		swapCreateInfo.queueFamilyIndexCount = (uint32_t)std::size(queueFamilyIndices);
		swapCreateInfo.pQueueFamilyIndices   = std::data(queueFamilyIndices);
	}
	swapCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapCreateInfo.minImageCount    = 2;
	swapCreateInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
	swapCreateInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapCreateInfo.surface          = m_primarySurface;
	swapCreateInfo.imageArrayLayers = 1;

	vkResult = vkCreateSwapchainKHR(context.Device, &swapCreateInfo, nullptr, &m_primarySwapChain);
	if(vkResult != VK_SUCCESS) {
		CR_LOG("Failed to create primary swap chain");
		DestroySwapChain();
		return false;
	}

	uint32_t numSwapChainImages{};
	vkGetSwapchainImagesKHR(context.Device, m_primarySwapChain, &numSwapChainImages, nullptr);
	m_primarySwapChainImages.resize(numSwapChainImages);
	vkGetSwapchainImagesKHR(context.Device, m_primarySwapChain, &numSwapChainImages,
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
		vkResult =
		    vkCreateImageView(context.Device, &viewInfo, nullptr, &m_primarySwapChainImageViews.back());
		if(vkResult != VK_SUCCESS) {
			CR_LOG("Failed to create primary swap chain image view");
			DestroySwapChain();
			return false;
		}
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

	vkResult = vkCreateRenderPass(context.Device, &renderPassInfo, nullptr, &m_renderPass);
	if(vkResult != VK_SUCCESS) {
		CR_LOG("Failed to create render pass");
		DestroySwapChain();
		return false;
	}

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
		vkResult = vkCreateFramebuffer(context.Device, &framebufferInfo, nullptr, &m_frameBuffers.back());
		if(vkResult != VK_SUCCESS) {
			CR_LOG("Failed to create frame buffer");
			DestroySwapChain();
			return false;
		}
	}
	return true;
}

void cegraph::DeviceService::DestroySwapChain() {
	const Context& context = GetContext();

	for(auto& framebuffer : m_frameBuffers) { vkDestroyFramebuffer(context.Device, framebuffer, nullptr); }
	m_frameBuffers.clear();

	if(m_renderPass) { vkDestroyRenderPass(context.Device, m_renderPass, nullptr); }
	m_renderPass = nullptr;

	for(auto& imageView : m_primarySwapChainImageViews) {
		vkDestroyImageView(context.Device, imageView, nullptr);
	}
	m_primarySwapChainImageViews.clear();
	m_primarySwapChainImages.clear();

	if(m_msaaView) { vkDestroyImageView(context.Device, m_msaaView, nullptr); }
	m_msaaView = nullptr;
	if(m_msaaAlloc) { vmaDestroyImage(context.Allocator, m_msaaImage, m_msaaAlloc); }
	m_msaaAlloc = nullptr;

	if(m_primarySwapChain) { vkDestroySwapchainKHR(context.Device, m_primarySwapChain, nullptr); }
	m_primarySwapChain = nullptr;
}

bool cegraph::DeviceService::ReInitialize() {
	const Context& context = GetContext();

	vkDeviceWaitIdle(context.Device);

	DestroySwapChain();

	return CreateSwapChain(context);
}