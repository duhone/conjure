module;

#include <function2/function2.hpp>
#include <glm/glm.hpp>

export module CR.Engine.Platform.Window;

import <cstdint>;
import <functional>;
import <memory>;
import <mutex>;
import <string>;
import <vector>;

namespace CR::Engine::Platform {
	// For now their is an assumption that an application only ever creates one of these.
	// Some work needs to be done if that assumption isn't valid.
	export class Window final {
	  public:
		using OnDestroy_t = fu2::unique_function<void()>;

		Window() = default;
		Window(std::string_view a_windowTitle, uint32_t a_width, uint32_t a_height, OnDestroy_t a_onDestroy);
		~Window();
		Window(const Window&)            = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&& a_other) noexcept;
		Window& operator=(Window&& a_other) noexcept;

		void Destroy();    // Will happen automatically on window destruction as well

		// call on main frame once per frame. as last as possible to minimize latency;
		void UpdateInput();

		// OS States are for use in Editors, or non full screen apps, or anytime you need to make sure to line
		// up to the onscreen cursor. Os States have OS processing applied, like mouse acceleration ect. OS
		// States have higher latency though.
		struct MouseStateOS {
			bool LeftDown{};
			glm::ivec2 Position{};
		};

		const MouseStateOS& GetMouseStateOS() const { return m_mouseState; }

		// Below needs to be public(for now) but is not intended to be called by clients;
		void OnDestroy();
		struct WindowData* GetWindowData() {
			return m_data.get();
		}

		// Windows specific, cast back as needed
		void* GetHInstance() const;
		void* GetHWND() const;

	  private:
		void UpdateInputPlatform();

		// WindowData is platform specific data
		std::unique_ptr<struct WindowData> m_data;

		MouseStateOS m_mouseState;
	};
}    // namespace CR::Engine::Platform

module :private;

namespace ceplat = CR::Engine::Platform;

void ceplat::Window::UpdateInput() {
	UpdateInputPlatform();
}
