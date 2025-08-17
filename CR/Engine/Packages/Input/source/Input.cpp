module;

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

module CR.Engine.Input;

import CR.Engine.Core;

import CR.Engine.Input.Context;

namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;

namespace {
	std::mutex m_inputLock;
	bool m_mouseAvailable{};
	bool m_mouseDown{};
	glm::dvec2 m_mousePos{};

	void cursorEnterCallback(GLFWwindow*, int a_entered) {
		std::scoped_lock lock(m_inputLock);
		if(a_entered) {
			m_mouseAvailable = true;
			m_mouseDown      = false;
		} else {
			m_mouseAvailable = false;
		}
	}

	void cursorPositionCallback(GLFWwindow*, double a_x, double a_y) {
		std::scoped_lock lock(m_inputLock);
		m_mousePos = {a_x, a_y};
	}

	void mouseButtonCallback(GLFWwindow*, int a_button, int a_action, int) {
		std::scoped_lock lock(m_inputLock);
		if(a_button == GLFW_MOUSE_BUTTON_LEFT) {
			if(a_action == GLFW_PRESS) {
				m_mouseDown = true;
			} else if(a_action == GLFW_RELEASE) {
				m_mouseDown = false;
			}
		}
	}
}    // namespace

void ceinput::Initialize(GLFWwindow* a_window) {
	glfwSetInputMode(a_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glfwSetCursorEnterCallback(a_window, cursorEnterCallback);
	glfwSetCursorPosCallback(a_window, cursorPositionCallback);
	glfwSetMouseButtonCallback(a_window, mouseButtonCallback);

	Regions::initialize();
}

void ceinput::Update() {
	std::scoped_lock lock(m_inputLock);
	auto& context = getContext();

	// we only have a cursor mode for mouse at the moment.

	bool mouseWasAvailable = (context.CursorState & CursorStates::Available) != 0;
	bool mouseWasDown      = (context.CursorState & CursorStates::Down) != 0;
	context.CursorState    = 0;
	if(m_mouseAvailable) {
		if(!mouseWasAvailable) {
			context.CursorState = CursorStates::Available;
			mouseWasDown        = false;
		}
		if(m_mouseDown) {
			context.CursorState |= CursorStates::Down;
			if(!mouseWasDown) { context.CursorState |= CursorStates::Pressed; }
		} else {
			if(mouseWasDown) { context.CursorState |= CursorStates::Released; }
		}
		context.CursorPos = m_mousePos;
	}
}

void ceinput::Shutdown() {}
