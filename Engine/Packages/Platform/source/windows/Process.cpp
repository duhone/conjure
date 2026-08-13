module;

#include "CRWindows.h"

#include <core/Core.h>

module CR.Engine.Platform.Process;

import CR.Engine.Core;

import std;

namespace CR::Engine::Platform {
	struct ProcessData {
		HANDLE m_processHandle;
		uint32_t m_processID;
	};
}    // namespace CR::Engine::Platform

namespace cep = CR::Engine::Platform;

cep::Process::Process(const char* a_executablePath, const char* a_commandLine) : m_data(new ProcessData) {
	STARTUPINFO startupInfo;
	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInfo;
	memset(&processInfo, 0, sizeof(processInfo));

	std::string commandLine = a_executablePath;
	commandLine += " ";
	commandLine += a_commandLine;
	auto created = CreateProcess(nullptr, (LPSTR)commandLine.c_str(), nullptr, nullptr, false, 0, nullptr,
	                             nullptr, &startupInfo, &processInfo);
	CR_ASSERT(created, "Failed to create process {}", a_executablePath);
	CloseHandle(processInfo.hThread);

	m_data->m_processHandle = processInfo.hProcess;
	m_data->m_processID     = processInfo.dwProcessId;
}

cep::Process::~Process() {
	if(m_data) { CloseHandle(m_data->m_processHandle); }
}

cep::Process::Process(Process&& a_other) noexcept {
	*this = std::move(a_other);
}

cep::Process& cep::Process::operator=(Process&& a_other) noexcept {
	m_data = std::move(a_other.m_data);
	return *this;
}

bool cep::Process::WaitForClose(const std::chrono::milliseconds& a_maxWait) {
	return WaitForSingleObject(m_data->m_processHandle, static_cast<DWORD>(a_maxWait.count())) ==
	       WAIT_OBJECT_0;
}

std::optional<int32_t> cep::Process::GetExitCode() const {
	int32_t exitCode = 0;
	if(GetExitCodeProcess(m_data->m_processHandle, (DWORD*)&exitCode) == FALSE) { return std::nullopt; }
	if(exitCode == STILL_ACTIVE) { return std::optional<int32_t>{}; }
	return std::optional<int32_t>{exitCode};
}
