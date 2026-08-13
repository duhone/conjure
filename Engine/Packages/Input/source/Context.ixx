module;

#include <glm/glm.hpp>

#include "core/Core.h"

export module CR.Engine.Input.Context;

import CR.Engine.Core;

import std;
import std.compat;

export namespace CR::Engine::Input {
	namespace CursorStates {
		// Other cursor states and pos only valid if available
		constexpr uint32_t Available = 1 << 0;
		// If pressed or released the previous frame.
		constexpr uint32_t Pressed  = 1 << 1;
		constexpr uint32_t Released = 1 << 2;
		// Current state, either down or up if this flag isn't there.
		constexpr uint32_t Down = 1 << 3;
	}    // namespace CursorStates

	struct Context {
		uint32_t CursorState{};
		glm::vec2 CursorPos;
	};

	Context& getContext();
}    // namespace CR::Engine::Input

module :private;

namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;

ceinput::Context& ceinput::getContext() {
	static ceinput::Context context;
	return context;
}