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
	// GraphicsThread promisses to execute tasks in order. Also each task will be 100% completed on the GPU
	// before the next task begins.
	export class GraphicsThread {
	  public:
		using taskSimple_t      = fu2::unique_function<void()>;
		using taskGPUCommands_t = fu2::unique_function<void(VkCommandBuffer& buffer)>;

		GraphicsThread(VkDevice a_device, VkQueue& a_transferQueue, uint32_t a_transferQueueFamily);
		~GraphicsThread();
		GraphicsThread(const GraphicsThread&)            = delete;
		GraphicsThread(GraphicsThread&&)                 = delete;
		GraphicsThread& operator=(const GraphicsThread&) = delete;
		GraphicsThread& operator=(GraphicsThread&&)      = delete;

		// For simple tasks that don't need to issue GPU commands. Compile a shader/ect.
		std::shared_ptr<std::atomic_bool> EnqueueTask(taskSimple_t&& a_task);
		// For tasks that need to issue GPU commands. The result won't be set until after the task has
		// completed on the GPU.
		std::shared_ptr<std::atomic_bool> EnqueueTask(taskGPUCommands_t&& a_task);

	  private:
		void ThreadMain();

		VkQueue& m_transferQueue;
		CommandPool m_commandPool;
		std::jthread m_thread;
		std::atomic_bool m_running;
		std::mutex m_requestMutex;
		std::condition_variable m_notify;

		struct Request {
			// This is either the complete task, or if ComplexTask exists then this is the completion task.
			taskSimple_t SimpleTask;
			taskGPUCommands_t ComplexTask;
			std::shared_ptr<std::atomic_bool> Completed;
		};
		std::deque<Request> m_requests;
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

namespace {

}

cegraph::GraphicsThread::GraphicsThread(VkDevice a_device, VkQueue& a_transferQueue,
                                        uint32_t a_transferQueueFamily) :
    m_transferQueue(a_transferQueue) {
	m_commandPool = CommandPool(a_device, a_transferQueueFamily);

	m_running.store(true, std::memory_order_release);
	m_thread = std::jthread([this]() { ThreadMain(); });
}

cegraph::GraphicsThread::~GraphicsThread() {
	m_running.store(false, std::memory_order_release);
	m_notify.notify_one();
	m_thread.join();
	m_commandPool.ResetAll();
}

void cegraph::GraphicsThread::ThreadMain() {
	while(m_running.load(std::memory_order_acquire)) {
		Request request;
		{
			std::unique_lock<std::mutex> lock(m_requestMutex);
			if(m_requests.empty()) { m_notify.wait(lock); }

			if(!m_requests.empty()) {
				request = std::move(m_requests.front());
				m_requests.pop_front();
			}
		}
		if(request.ComplexTask) {
			VkCommandBuffer buffer = m_commandPool.Begin();
			request.ComplexTask(buffer);
			m_commandPool.End(buffer);

			VkSubmitInfo subInfo;
			ClearStruct(subInfo);
			subInfo.commandBufferCount = 1;
			subInfo.pCommandBuffers    = &buffer;
			vkQueueSubmit(m_transferQueue, 1, &subInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(m_transferQueue);
			m_commandPool.ResetAll();

			request.Completed->store(true, std::memory_order_release);
			request = Request{};

		} else if(request.SimpleTask) {
			request.SimpleTask();
			request.Completed->store(true, std::memory_order_release);
			request = Request{};
		}
	}
	m_requests.clear();
}

std::shared_ptr<std::atomic_bool> cegraph::GraphicsThread::EnqueueTask(taskSimple_t&& a_task) {
	std::shared_ptr<std::atomic_bool> result = std::make_shared<std::atomic_bool>(false);

	{
		std::unique_lock<std::mutex> lock(m_requestMutex);
		m_requests.push_back({std::move(a_task), nullptr, result});
	}
	m_notify.notify_one();
	return result;
}

std::shared_ptr<std::atomic_bool> cegraph::GraphicsThread::EnqueueTask(taskGPUCommands_t&& a_task) {
	std::shared_ptr<std::atomic_bool> result = std::make_shared<std::atomic_bool>(false);

	{
		std::unique_lock<std::mutex> lock(m_requestMutex);
		m_requests.push_back({nullptr, std::move(a_task), result});
	}
	m_notify.notify_one();
	return result;
}