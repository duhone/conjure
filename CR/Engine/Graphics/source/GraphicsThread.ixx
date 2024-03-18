module;

#include "core/Log.h"

#include "Core.h"

#include <function2/function2.hpp>

export module CR.Engine.Graphics.GraphicsThread;

import CR.Engine.Core;
import CR.Engine.Graphics.CommandPool;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Utils;

import <deque>;
import <thread>;
import <atomic>;
import <mutex>;

namespace cecore = CR::Engine::Core;

namespace CR::Engine::Graphics {
	// GraphicsThread promises to execute tasks in order. Also each task will be 100% completed on the GPU
	// before the next task begins.
	export namespace GraphicsThread {
		using taskSimple_t      = fu2::unique_function<void()>;
		using taskGPUCommands_t = fu2::unique_function<void(VkCommandBuffer& buffer)>;

		void Initialize(VkDevice a_device, VkQueue& a_transferQueue, uint32_t a_transferQueueFamily);
		void Shutdown();

		// For simple tasks that don't need to issue GPU commands. Compile a shader/ect.
		// a_completed will be set to true once the task has finished. It is callers responsibility to ensure
		// a_completed is still alive until the task completes.
		void EnqueueTask(taskSimple_t&& a_task, std::atomic_bool& a_completed);
		// For tasks that need to issue GPU commands. a_completed won't be set until after all GPU commands
		// added to the command buffer have completed on the GPU as well.
		void EnqueueTask(taskGPUCommands_t&& a_task, std::atomic_bool& a_completed);

	};    // namespace GraphicsThread
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

namespace {
	struct Request {
		// This is either the complete task, or if ComplexTask exists then this is the completion task.
		cegraph::GraphicsThread::taskSimple_t SimpleTask;
		cegraph::GraphicsThread::taskGPUCommands_t ComplexTask;
		std::atomic_bool* Completed{nullptr};
	};

	struct Data {
		VkQueue& TransferQueue;
		cegraph::CommandPool CommandPool;
		std::jthread Thread;
		std::atomic_bool Running;
		std::mutex RequestMutex;
		std::condition_variable Notify;

		std::deque<Request> Requests;
	};
	Data* g_data = nullptr;

	void ThreadMain() {
		CR_ASSERT(g_data != nullptr, "Graphics thread not initialized");
		while(g_data->Running.load(std::memory_order_acquire)) {
			Request request;
			{
				std::unique_lock<std::mutex> lock(g_data->RequestMutex);
				if(g_data->Requests.empty()) { g_data->Notify.wait(lock); }

				if(!g_data->Requests.empty()) {
					request = std::move(g_data->Requests.front());
					g_data->Requests.pop_front();
				}
			}
			if(request.ComplexTask) {
				// CR_ASSERT(request.Completed != nullptr, "Completed should never be null");
				VkCommandBuffer buffer = g_data->CommandPool.Begin();
				request.ComplexTask(buffer);
				g_data->CommandPool.End(buffer);

				VkSubmitInfo subInfo;
				cegraph::ClearStruct(subInfo);
				subInfo.commandBufferCount = 1;
				subInfo.pCommandBuffers    = &buffer;
				vkQueueSubmit(g_data->TransferQueue, 1, &subInfo, VK_NULL_HANDLE);
				vkQueueWaitIdle(g_data->TransferQueue);
				g_data->CommandPool.ResetAll();

				request.Completed->store(true, std::memory_order_release);
				request = Request{};

			} else if(request.SimpleTask) {
				// CR_ASSERT(request.Completed != nullptr, "Completed should never be null");
				request.SimpleTask();
				request.Completed->store(true, std::memory_order_release);
				request = Request{};
			}
		}
		g_data->Requests.clear();
	}
}    // namespace

void cegraph::GraphicsThread::Initialize(VkDevice a_device, VkQueue& a_transferQueue,
                                         uint32_t a_transferQueueFamily) {
	CR_ASSERT(g_data == nullptr, "GraphicsThread is already initialized");
	g_data = new Data{a_transferQueue};

	g_data->CommandPool = CommandPool(a_device, a_transferQueueFamily);

	g_data->Running.store(true, std::memory_order_release);
	g_data->Thread = std::jthread(ThreadMain);
}

void cegraph::GraphicsThread::Shutdown() {
	CR_ASSERT(g_data != nullptr, "GraphicsThread not initialized");

	g_data->Running.store(false, std::memory_order_release);
	g_data->Notify.notify_one();
	g_data->Thread.join();
	g_data->CommandPool.ResetAll();

	delete g_data;
}

void cegraph::GraphicsThread::EnqueueTask(taskSimple_t&& a_task, std::atomic_bool& a_completed) {
	{
		std::unique_lock<std::mutex> lock(g_data->RequestMutex);
		g_data->Requests.push_back({std::move(a_task), nullptr, &a_completed});
	}
	g_data->Notify.notify_one();
}

void cegraph::GraphicsThread::EnqueueTask(taskGPUCommands_t&& a_task, std::atomic_bool& a_completed) {
	{
		std::unique_lock<std::mutex> lock(g_data->RequestMutex);
		g_data->Requests.push_back({nullptr, std::move(a_task), &a_completed});
	}
	g_data->Notify.notify_one();
}