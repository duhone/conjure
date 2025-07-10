module;

#include "core/Log.h"
#include <platform/windows/CRWindows.h>

module CR.Engine.Platform.SharedLibrary;

namespace CR::Engine::Platform {
	struct SharedLibraryData {
		HMODULE m_libraryModule;
	};
}    // namespace CR::Engine::Platform

namespace cep = CR::Engine::Platform;

/*
#include <strsafe.h>
void ErrorExit(LPTSTR lpszFunction)
{
  // Retrieve the system error message for the last-error code

  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;
  DWORD dw = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR)&lpMsgBuf,
    0, NULL);

  // Display the error message and exit the process

  lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
                    (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
  StringCchPrintf((LPTSTR)lpDisplayBuf,
          LocalSize(lpDisplayBuf) / sizeof(TCHAR),
          TEXT("%s failed with error %d: %s"),
          lpszFunction, dw, lpMsgBuf);
  MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
  //ExitProcess(dw);
}
*/

cep::SharedLibrary::SharedLibrary(std::string_view a_libraryName) : m_data(new SharedLibraryData) {
	m_data->m_libraryModule = LoadLibrary(std::string(a_libraryName).c_str());
	if(!m_data->m_libraryModule) {
		// ErrorExit("Failed to load library");
		CR_ERROR("Failed to load library");
	}
}

cep::SharedLibrary::~SharedLibrary() {
	if(!m_data) { return; }
	FreeLibrary(m_data->m_libraryModule);
}

void* cep::SharedLibrary::GetFunction(std::string_view a_functionName) const {
	return GetProcAddress(m_data->m_libraryModule, std::string(a_functionName).c_str());
}

cep::SharedLibrary::SharedLibrary(SharedLibrary&& a_other) noexcept {
	*this = std::move(a_other);
}

cep::SharedLibrary& cep::SharedLibrary::operator=(SharedLibrary&& a_other) noexcept {
	m_data = std::move(a_other.m_data);
	return *this;
}
