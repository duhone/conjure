module;

#include <platform/windows/CRWindows.h>

#include "core/Log.h"
#include <function2/function2.hpp>

export module CR.Engine.Platform.IOCP;

import <thread>;

namespace CR::Engine::Platform {
	export class IOCPPort final {
	  public:
		using CompletionCallback_t = fu2::unique_function<void(OVERLAPPED*, std::size_t)>;

		IOCPPort() = default;
		IOCPPort(HANDLE a_handle, CompletionCallback_t a_completion);
		~IOCPPort()                          = default;
		IOCPPort(const IOCPPort&)            = delete;
		IOCPPort& operator=(const IOCPPort&) = delete;
		IOCPPort(IOCPPort&& a_other) noexcept;
		IOCPPort& operator=(IOCPPort&& a_other) noexcept;

		void RunContinuation(OVERLAPPED* a_result, std::size_t a_msgSize) {
			m_completion(a_result, a_msgSize);
		}

	  private:
		CompletionCallback_t m_completion;
	};

}    // namespace CR::Engine::Platform

module :private;

namespace {
	// Doesn't scale past one thread at the moment. would require some redesign to do so.
	class IOCPThread final {
	  public:
		void RegisterIOCPPort(CR::Engine::Platform::IOCPPort* a_port, HANDLE a_handle);
		IOCPThread();
		~IOCPThread();
		IOCPThread(const IOCPThread&)            = delete;
		IOCPThread& operator=(const IOCPThread&) = delete;
		IOCPThread(IOCPThread&&)                 = delete;
		IOCPThread& operator=(IOCPThread&&)      = delete;

		void RunIOCPThread();

	  private:
		std::thread m_iocpThread;
		HANDLE m_iocpHandle;
	};

	IOCPThread& GetIOCPThread() {
		static IOCPThread iocpThread;
		return iocpThread;
	}
}    // namespace

IOCPThread::IOCPThread() {
	m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
	m_iocpThread = std::thread{[this]() { this->RunIOCPThread(); }};
}

IOCPThread::~IOCPThread() {
	CloseHandle(m_iocpHandle);
	if(m_iocpThread.joinable()) { m_iocpThread.join(); }
}

void IOCPThread::RunIOCPThread() {
	DWORD msgSize;
	ULONG_PTR completionKey;
	OVERLAPPED* msg;
	while((bool)GetQueuedCompletionStatus(m_iocpHandle, &msgSize, &completionKey, &msg, INFINITE)) {
		((CR::Engine::Platform::IOCPPort*)completionKey)->RunContinuation(msg, msgSize);
	}
}

void IOCPThread::RegisterIOCPPort(CR::Engine::Platform::IOCPPort* a_port, HANDLE a_handle) {
	CreateIoCompletionPort(a_handle, m_iocpHandle, (ULONG_PTR)a_port, 1);
}

CR::Engine::Platform::IOCPPort::IOCPPort(HANDLE a_handle, CompletionCallback_t a_completion) :
    m_completion(std::move(a_completion)) {
	GetIOCPThread().RegisterIOCPPort(this, a_handle);
}

inline CR::Engine::Platform::IOCPPort::IOCPPort(IOCPPort&& a_other) noexcept {
	*this = std::move(a_other);
}

inline CR::Engine::Platform::IOCPPort&
    CR::Engine::Platform::IOCPPort::operator=(IOCPPort&& a_other) noexcept {
	m_completion = std::move(a_other.m_completion);
	return *this;
}
