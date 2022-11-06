module;

#include "core/Log.h"

#include <function2/function2.hpp>

export module CR.Engine.Input.Service;

import CR.Engine.Core;
import CR.Engine.Platform;

import <typeindex>;

namespace CR::Engine::Input {
	export class Service {
	  public:
		static std::type_index s_typeIndex;

		Service(CR::Engine::Platform::Window& a_window);
		~Service()              = default;
		Service(const Service&) = delete;
		Service(Service&&)      = delete;

		Service& operator=(const Service&) = delete;
		Service& operator=(Service&&)      = delete;

		void Update();

	  private:
		CR::Engine::Platform::Window& m_window;
	};
}    // namespace CR::Engine::Input

module :private;

namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;

std::type_index ceinput::Service::s_typeIndex{typeid(Service)};

ceinput::Service::Service(CR::Engine::Platform::Window& a_window) : m_window(a_window) {}

void ceinput::Service::Update() {}
