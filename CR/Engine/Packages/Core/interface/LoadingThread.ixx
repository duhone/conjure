module;

#include "core/Log.h"

export module CR.Engine.Core.LoadingThread;

import CR.Engine.Core.Log;

import std;

export namespace CR::Engine::Core::LoadingThread {
	using task_t = std::move_only_function<void()>;

	void EnqueueTask(task_t&& a_task, std::atomic_flag& a_completed);

	namespace Internal {
		void Initialize();
		void Shutdown();
	}    // namespace Internal
}    // namespace CR::Engine::Core::LoadingThread

module :private;

namespace cecore = CR::Engine::Core;

namespace {
	struct Request {
		cecore::LoadingThread::task_t Task;
		std::atomic_flag* Completed{nullptr};
	};

	std::jthread m_thread;
	std::atomic_bool m_running;
	std::mutex m_requestMutex;
	std::condition_variable m_notify;

	std::deque<Request> m_requests;

	void ThreadMain() {
		while(m_running.load(std::memory_order_acquire)) {
			Request request;
			{
				std::unique_lock<std::mutex> lock(m_requestMutex);
				if(m_requests.empty()) { m_notify.wait(lock); }

				if(!m_requests.empty() && m_running.load(std::memory_order_acquire)) {
					request = std::move(m_requests.front());
					m_requests.pop_front();
				}
			}
			if(request.Task) {
				CR_ASSERT(request.Completed != nullptr, "Completed should never be null");
				request.Task();
				request.Completed->test_and_set(std::memory_order_release);
				request.Completed->notify_all();
				request = Request{};
			}
		}
		m_requests.clear();
	}
}    // namespace

void cecore::LoadingThread::Internal::Initialize() {
	m_running.store(true, std::memory_order_release);
	m_thread = std::jthread(ThreadMain);
}

void cecore::LoadingThread::Internal::Shutdown() {
	m_running.store(false, std::memory_order_release);
	m_notify.notify_one();
	m_thread.join();
}

void cecore::LoadingThread::EnqueueTask(task_t&& a_task, std::atomic_flag& a_completed) {
	{
		std::unique_lock<std::mutex> lock(m_requestMutex);
		m_requests.push_back({std::move(a_task), &a_completed});
	}
	m_notify.notify_one();
}
