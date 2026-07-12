module;

#include "core/Log.h"

#include "Core.h"

export module CR.Engine.Graphics.GraphicsThread;

import CR.Engine.Core;
import CR.Engine.Graphics.CommandPool;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Utils;

import std;
import std.compat;

namespace cecore = CR::Engine::Core;

namespace CR::Engine::Graphics {
	// GraphicsThread promises to execute tasks in order. Also each task will be 100% completed on the GPU
	// before the next task begins.
	export namespace GraphicsThread {
		using taskSimple_t      = std::move_only_function<void()>;
		using taskGPUCommands_t = std::move_only_function<void(VkCommandBuffer& buffer)>;

		void Initialize(VkQueue& a_transferQueue);
		void Shutdown();

		// For simple tasks that don't need to issue GPU commands. Compile a shader/ect.
		// a_completed will be set to true once the task has finished. It is callers responsibility to ensure
		// a_completed is still alive until the task completes.
		void EnqueueTask(taskSimple_t&& a_task, std::atomic_flag& a_completed);
		// For tasks that need to issue GPU commands. a_completed won't be set until after all GPU commands
		// added to the command buffer have completed on the GPU as well.
		void EnqueueTask(taskGPUCommands_t&& a_task, std::atomic_flag& a_completed);

	};    // namespace GraphicsThread
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

namespace {
	struct Request {
		// This is either the complete task, or if ComplexTask exists then this is the completion task.
		cegraph::GraphicsThread::taskSimple_t SimpleTask;
		cegraph::GraphicsThread::taskGPUCommands_t ComplexTask;
		std::atomic_flag* Completed{nullptr};
	};

	VkQueue* m_transferQueue{};
	cegraph::Handles::CommandPool m_commandPool;
	std::jthread m_thread;
	std::atomic_bool m_running;
	std::mutex m_requestMutex;
	std::condition_variable m_notify;

	std::deque<Request> m_requests;

	void ThreadMain() {
		CR_ASSERT(m_transferQueue != nullptr, "Graphics thread not initialized");
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
				CR_ASSERT(request.Completed != nullptr, "Completed should never be null");
				auto buffer = cegraph::CommandPools::Begin(m_commandPool);
				request.ComplexTask(buffer);
				cegraph::CommandPools::End(buffer);

				VkSubmitInfo subInfo;
				cegraph::ClearStruct(subInfo);
				subInfo.commandBufferCount = 1;
				subInfo.pCommandBuffers    = &buffer;
				vkQueueSubmit(*m_transferQueue, 1, &subInfo, VK_NULL_HANDLE);
				vkQueueWaitIdle(*m_transferQueue);
				cegraph::CommandPools::ResetAll(m_commandPool);

				request.Completed->test_and_set(std::memory_order_release);
				request.Completed->notify_all();
				request = Request{};

			} else if(request.SimpleTask) {
				// CR_ASSERT(request.Completed != nullptr, "Completed should never be null");
				request.SimpleTask();
				request.Completed->test_and_set(std::memory_order_release);
				request.Completed->notify_all();
				request = Request{};
			}
		}
		m_requests.clear();
	}
}    // namespace

void cegraph::GraphicsThread::Initialize(VkQueue& a_transferQueue) {
	CR_ASSERT(m_transferQueue == nullptr, "GraphicsThread is already initialized");
	m_transferQueue = &a_transferQueue;

	m_commandPool = cegraph::CommandPools::Create(GetContext().TransferQueueIndex);

	m_running.store(true, std::memory_order_release);
	m_thread = std::jthread(ThreadMain);
}

void cegraph::GraphicsThread::Shutdown() {
	CR_ASSERT(m_transferQueue != nullptr, "GraphicsThread not initialized");

	m_running.store(false, std::memory_order_release);
	m_notify.notify_one();
	m_thread.join();
	cegraph::CommandPools::ResetAll(m_commandPool);
	cegraph::CommandPools::Delete(m_commandPool);

	m_transferQueue = nullptr;
}

void cegraph::GraphicsThread::EnqueueTask(taskSimple_t&& a_task, std::atomic_flag& a_completed) {
	{
		std::unique_lock<std::mutex> lock(m_requestMutex);
		m_requests.push_back({std::move(a_task), nullptr, &a_completed});
	}
	m_notify.notify_one();
}

void cegraph::GraphicsThread::EnqueueTask(taskGPUCommands_t&& a_task, std::atomic_flag& a_completed) {
	{
		std::unique_lock<std::mutex> lock(m_requestMutex);
		m_requests.push_back({nullptr, std::move(a_task), &a_completed});
	}
	m_notify.notify_one();
}