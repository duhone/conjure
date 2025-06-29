module;

#include "core/Log.h"

#include <glm/glm.hpp>

export module CR.Engine.Graphics.Service;

import CR.Engine.Graphics.DeviceService;
import CR.Engine.Graphics.Handles;
import CR.Engine.Graphics.Textures;

import CR.Engine.Core;
import CR.Engine.Platform;

import <optional>;
import <span>;
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

		// If this returns false, then you must ReInitialize() the service. This can happen if the app was
		// resized for instance. May not be fool proof on your platform, you should also check OS for resize
		// as well.
		bool Update();
		// If this returns false, you can't call Update(). Maybe app was minimized? Check for that yourself.
		// ReInitialize again once you detect app isn't minimized, or just try again once in a while. Should
		// pause app while you can't render anything.
		bool ReInitialize();

		Handles::Texture GetHandle(uint64_t hash);
		Handles::TextureSet LoadTextureSet(std::span<uint64_t> hashes);
		void ReleaseTextureSet(Handles::TextureSet set);

	  private:
		DeviceService& m_deviceService;
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace cegraph = CR::Engine::Graphics;

cegraph::Service::Service(ceplat::Window& a_window, std::optional<glm::vec4> a_clearColor) :
    m_deviceService(cecore::AddService<DeviceService>(a_window, a_clearColor)) {}

bool cegraph::Service::Update() {
	return m_deviceService.Update();
}

bool cegraph::Service::ReInitialize() {
	return m_deviceService.ReInitialize();
}

cegraph::Handles::Texture cegraph::Service::GetHandle(uint64_t hash) {
	return CR::Engine::Graphics::Textures::GetHandle(hash);
}

cegraph::Handles::TextureSet cegraph::Service::LoadTextureSet(std::span<uint64_t> hashes) {
	return CR::Engine::Graphics::Textures::LoadTextureSet(hashes);
}

void cegraph::Service::ReleaseTextureSet(Handles::TextureSet set) {
	CR::Engine::Graphics::Textures::ReleaseTextureSet(set);
}
