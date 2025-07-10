module;

#include "core/Log.h"
#include <function2/function2.hpp>
#include <platform/windows/CRWindows.h>

module CR.Engine.Platform.PipeClient;

import CR.Engine.Core.Literals;

import <cassert>;
import <stdexcept>;
import <thread>;

namespace CR::Engine::Platform {
	struct PipeClientData {
		HANDLE m_pipeHandle;
		PipeClient::MsgHandlerT m_msgHandler;
		std::thread m_msgThread;
		std::atomic_bool m_closed{false};

		OVERLAPPED m_writeOverlapped;
		HANDLE m_writeEvent;
	};
}    // namespace CR::Engine::Platform

namespace cep = CR::Engine::Platform;

using namespace CR::Engine::Core::Literals;

cep::PipeClient::PipeClient(const char* a_name, PipeClient::MsgHandlerT a_msgHandler) :
    m_data(new PipeClientData{}) {
	CR_ASSERT((bool)a_msgHandler, "a_msgHandler is required");
	m_data->m_msgHandler = std::move(a_msgHandler);

	m_data->m_pipeHandle = CreateFile(a_name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
	                                  FILE_FLAG_OVERLAPPED, nullptr);
	CR_ASSERT(m_data->m_pipeHandle != INVALID_HANDLE_VALUE, "Couldn't open pipe handle with name {}", a_name);
	DWORD pipeState = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(m_data->m_pipeHandle, &pipeState, nullptr, nullptr);

	m_data->m_msgThread = std::thread([this]() { this->RunMsgHandler(); });

	memset(&m_data->m_writeOverlapped, 0, sizeof(m_data->m_writeOverlapped));
	m_data->m_writeEvent             = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	m_data->m_writeOverlapped.hEvent = m_data->m_writeEvent;
}

cep::PipeClient::~PipeClient() {
	if(!m_data) { return; }
	m_data->m_closed.store(true, std::memory_order_release);
	CloseHandle(m_data->m_pipeHandle);
	if(m_data->m_msgThread.joinable()) { m_data->m_msgThread.join(); }
	CloseHandle(m_data->m_writeEvent);
}

cep::PipeClient::PipeClient(PipeClient&& a_other) noexcept {
	*this = std::move(a_other);
}

cep::PipeClient& cep::PipeClient::operator=(PipeClient&& a_other) noexcept {
	m_data = std::move(a_other.m_data);
	return *this;
}

void cep::PipeClient::SendPipeMessage(const void* a_msg, size_t a_msgSize) {
	DWORD bytesWritten = 0;
	CR_ASSERT(a_msgSize < 4_KB,
	          "a_msgSize too large");    // arbitrary, but if larger than this might want to switch to
	                                     // an async implementation
	ResetEvent(m_data->m_writeEvent);
	BOOL result = WriteFile(m_data->m_pipeHandle, a_msg, static_cast<DWORD>(a_msgSize), nullptr,
	                        &m_data->m_writeOverlapped);
	auto error  = GetLastError();
	if(!result && error == ERROR_IO_PENDING) {
		if(GetOverlappedResult(m_data->m_pipeHandle, &m_data->m_writeOverlapped, &bytesWritten, TRUE) == 0) {
			CR_ERROR("Failed to write message");
		}
	}
}

void cep::PipeClient::RunMsgHandler() {
	char msg[4_KB];
	DWORD bytesRead;
	OVERLAPPED overlapped;
	memset(&overlapped, 0, sizeof(overlapped));
	HANDLE completeEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);
	overlapped.hEvent    = completeEvent;
	while(!m_data->m_closed.load(std::memory_order_acquire)) {
		ResetEvent(completeEvent);
		auto result = ReadFile(m_data->m_pipeHandle, msg, static_cast<DWORD>(4_KB), nullptr, &overlapped);
		auto error  = GetLastError();
		if(!result && error == ERROR_IO_PENDING) {
			if(GetOverlappedResult(m_data->m_pipeHandle, &overlapped, &bytesRead, TRUE) != 0) {
				m_data->m_msgHandler(msg, bytesRead);
			}
		}
	}
}
