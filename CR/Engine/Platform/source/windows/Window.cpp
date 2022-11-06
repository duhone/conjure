module;

#include "core/Log.h"
#include <platform/windows/CRWindows.h>

#include <function2/function2.hpp>
#include <glm/glm.hpp>

module CR.Engine.Platform.Window;

import CR.Engine.Core.Locked;
import <thread>;
import <unordered_map>;

namespace CR::Engine::Platform {
	struct WindowData {
		void MyCreateWindow(std::string_view a_windowTitle, uint32_t a_width, uint32_t a_height,
		                    Window* self);
		void RunMsgLoop();
		void UpdateMousePos(int a_x, int a_y);
		void UpdateLeftMouse(bool a_down, int a_x, int a_y);

		HWND m_HWND{nullptr};
		std::jthread m_thread;
		Window::OnDestroy_t m_onDestroy;

		std::mutex m_inputMutex;
		glm::ivec2 m_mousePos;
		bool m_mouseLeftDown{false};
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
			case WM_LBUTTONDOWN:
				g_windowLookup([&](auto& winLookup) {
					auto window = winLookup.find(a_hWnd);
					if(window != end(winLookup)) {
						auto winData = window->second->GetWindowData();

						SetCapture(a_hWnd);
						winData->UpdateLeftMouse(true, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
					}
				});
				break;
			case WM_MOUSEMOVE:
				g_windowLookup([&](auto& winLookup) {
					auto window = winLookup.find(a_hWnd);
					if(window != end(winLookup)) {
						auto winData = window->second->GetWindowData();
						winData->UpdateMousePos(GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
					}
				});
				break;
			case WM_LBUTTONUP:
				g_windowLookup([&](auto& winLookup) {
					auto window = winLookup.find(a_hWnd);
					if(window != end(winLookup)) {
						auto winData = window->second->GetWindowData();

						SetCapture(nullptr);
						winData->UpdateLeftMouse(false, GET_X_LPARAM(a_lParam), GET_Y_LPARAM(a_lParam));
					}
				});
				break;
		}

		// Handle any messages the switch statement didn't.
		return DefWindowProc(a_hWnd, a_message, a_wParam, a_lParam);
	}
}    // namespace

cep::Window::Window(std::string_view a_windowTitle, uint32_t a_width, uint32_t a_height,
                    OnDestroy_t a_onDestroy) {
	m_data              = std::make_unique<WindowData>();
	m_data->m_onDestroy = std::move(a_onDestroy);
	m_data->m_thread    = std::jthread([this, windowTitle = std::string{a_windowTitle}, a_width, a_height]() {
        this->m_data->MyCreateWindow(windowTitle.c_str(), a_width, a_height, this);
        this->m_data->RunMsgLoop();
    });

	ShowWindow(GetConsoleWindow(), SW_HIDE);
}

cep::Window::~Window() {
	if(!m_data) { return; }
	g_windowLookup([this](WindowLookup_t& winLookup) { winLookup.erase(m_data->m_HWND); });
	Destroy();
	ShowWindow(GetConsoleWindow(), SW_SHOW);
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

void cep::Window::UpdateInputPlatform() {
	CR_ASSERT(m_data, "platform data not created");
	std::scoped_lock lock(m_data->m_inputMutex);
	m_mouseState.LeftDown = m_data->m_mouseLeftDown;
	m_mouseState.Position = m_data->m_mousePos;
}

void cep::WindowData::MyCreateWindow(std::string_view a_windowTitle, uint32_t a_width, uint32_t a_height,
                                     Window* self) {
	// Initialize the window class.
	WNDCLASSEX windowClass    = {0};
	windowClass.cbSize        = sizeof(WNDCLASSEX);
	windowClass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
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

void cep::WindowData::UpdateMousePos(int a_x, int a_y) {
	std::scoped_lock lock(m_inputMutex);
	m_mousePos.x = a_x;
	m_mousePos.y = a_y;
}

void cep::WindowData::UpdateLeftMouse(bool a_down, int a_x, int a_y) {
	std::scoped_lock lock(m_inputMutex);
	m_mouseLeftDown = a_down;
	m_mousePos.x    = a_x;
	m_mousePos.y    = a_y;
}