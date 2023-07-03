module;

#include "core/Log.h"

#include <glm/glm.hpp>

export module CR.Engine.Graphics.Service;

import CR.Engine.Graphics.DeviceService;

import CR.Engine.Core;
import CR.Engine.Platform;

import <optional>;
import <typeindex>;

namespace cecore = CR::Engine::Core;
namespace ceplat = CR::Engine::Platform;

namespace CR::Engine::Graphics {
	export class Service {
	  public:
		static inline constexpr uint64_t s_typeIndex = CR::Engine::Core::EightCC("EGraServ");

		Service(ceplat::Window& a_window, std::optional<glm::vec4> a_clearColor);
		~Service()              = default;
		Service(const Service&) = delete;
		Service(Service&&)      = delete;

		Service& operator=(const Service&) = delete;
		Service& operator=(Service&&)      = delete;

		void Update();

	  private:
		DeviceService& m_deviceService;
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

cegraph::Service::Service(ceplat::Window& a_window, std::optional<glm::vec4> a_clearColor) :
    m_deviceService(cecore::AddService<DeviceService>(a_window, a_clearColor)) {}

void cegraph::Service::Update() {
	m_deviceService.Update();
}
