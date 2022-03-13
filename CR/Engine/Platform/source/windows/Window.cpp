module;

#include "core/Log.h"
#include <platform/windows/CRWindows.h>

#include <function2/function2.hpp>

module CR.Engine.Platform.Window;

import CR.Engine.Core.Locked;
import<thread>;
import<unordered_map>;

namespace CR::Engine::Platform {
	struct WindowData {
		void MyCreateWindow(std::string_view a_windowTitle, uint32_t a_width, uint32_t a_height,
		                    Window* self);
		void RunMsgLoop();

		HWND m_HWND{nullptr};
		std::thread m_thread;
		Window::OnDestroy_t m_onDestroy;
	};
}    // namespace CR::Engine::Platform

namespace cec = CR::Engine::Core;
namespace cep = CR::Engine::Platform;

namespace {
	using WindowLookup_t = std::unordered_map<HWND, cep::Window*>;
	cec::Locked<WindowLookup_t> g_windowLookup;

	LRESULT CALLBACK WinProc(HWND a_hWnd, UINT a_message, WPARAM a_wParam, LPARAM a_lParam) {
		switch(a_message) {
			case WM_DESTROY:
				g_windowLookup([a_hWnd](auto& winLookup) {
					auto window = winLookup.find(a_hWnd);
					if(window != end(winLookup)) { window->second->OnDestroy(); }
				});
				PostQuitMessage(0);
				return 0;
		}

		// Handle any messages the switch statement didn't.
		return DefWindowProc(a_hWnd, a_message, a_wParam, a_lParam);
	}
}    // namespace

cep::Window::Window(std::string_view a_windowTitle, uint32_t a_width, uint32_t a_height,
                    OnDestroy_t a_onDestroy) {
	m_data->m_onDestroy = std::move(a_onDestroy);
	m_data->m_thread    = std::thread([this, windowTitle = std::string{a_windowTitle}, a_width, a_height]() {
        this->m_data->MyCreateWindow(windowTitle.c_str(), a_width, a_height, this);
        this->m_data->RunMsgLoop();
	   });
}

cep::Window::~Window() {
	if(!m_data) { return; }
	g_windowLookup([this](WindowLookup_t& winLookup) { winLookup.erase(m_data->m_HWND); });
	Destroy();
	if(m_data->m_thread.joinable()) { m_data->m_thread.join(); }
}

cep::Window::Window(Window&& a_other) noexcept {
	*this = std::move(a_other);
}

cep::Window& cep::Window::operator=(Window&& a_other) noexcept {
	m_data = std::move(a_other.m_data);
	return *this;
}

void cep::Window::Destroy() {
	PostMessage(m_data->m_HWND, WM_DESTROY, 0, 0);
}

void cep::Window::OnDestroy() {
	m_data->m_onDestroy();
}

void cep::WindowData::MyCreateWindow(std::string_view a_windowTitle, uint32_t a_width, uint32_t a_height,
                                     Window* self) {
	// Initialize the window class.
	WNDCLASSEX windowClass    = {0};
	windowClass.cbSize        = sizeof(WNDCLASSEX);
	windowClass.style         = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc   = WinProc;
	windowClass.hInstance     = GetModuleHandle(NULL);
	windowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "WindowClass1";
	RegisterClassEx(&windowClass);

	RECT windowRect = {0, 0, static_cast<LONG>(a_width), static_cast<LONG>(a_height)};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	m_HWND = CreateWindowEx(NULL, "WindowClass1", std::string(a_windowTitle).c_str(), WS_OVERLAPPEDWINDOW,
	                        CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left,
	                        windowRect.bottom - windowRect.top, NULL, NULL, GetModuleHandle(NULL), NULL);

	ShowWindow(m_HWND, TRUE);

	g_windowLookup([this, self](WindowLookup_t& winLookup) { winLookup[m_HWND] = self; });
}

void cep::WindowData::RunMsgLoop() {
	MSG msg           = {0};
	BOOL GetMsgResult = TRUE;
	while((GetMsgResult = GetMessage(&msg, NULL, 0, 0)) != 0) {
		if(GetMsgResult < 0) { PostQuitMessage(GetMsgResult); }

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
