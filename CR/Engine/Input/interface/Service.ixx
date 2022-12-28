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
		static inline constexpr uint64_t s_typeIndex = CR::Engine::Core::EightCC("EInpServ");

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

import CR.Engine.Input.RegionService;

namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;

ceinput::Service::Service(CR::Engine::Platform::Window& a_window) : m_window(a_window) {
	cecore::AddService<RegionService>();
}

void ceinput::Service::Update() {}
