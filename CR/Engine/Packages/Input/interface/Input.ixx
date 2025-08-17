module;

#include <GLFW/glfw3.h>

export module CR.Engine.Input;

export import CR.Engine.Input.Handles;
export import CR.Engine.Input.Regions;

export namespace CR::Engine::Input {
	void Initialize(GLFWwindow* a_window);
	// should be called after glfwPollEvents
	void Update();
	void Shutdown();
}    // namespace CR::Engine::Input
