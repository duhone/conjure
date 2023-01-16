module;

#include "core/Log.h"

export module CR.Engine.Input.Regions;

import CR.Engine.Core;
import CR.Engine.Input.RegionService;

import <cstdint>;

namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;

namespace CR::Engine::Input {
	export class Regions;

	export class Region final {
		friend Regions;

	  public:
		Region() = default;
		~Region() {
			if(!m_service) { return; }
			m_service->Delete(m_id);
		}

		void Update(const cecore::Rect2Di32& a_value) {
			CR_ASSERT_AUDIT(m_service != nullptr, "trying to updatie an invalid Region");
			m_service->Update(m_id, a_value);
		}

		bool operator==(const Region& a_other) const {
			if(m_service == nullptr || a_other.m_service == nullptr) { return false; }
			return m_id == a_other.m_id;
		}
		bool operator!=(const Region& a_other) const = default;

	  private:
		Region(RegionService& a_service, std::uint16_t a_id) : m_service(&a_service), m_id(a_id) {}

		RegionService* m_service{};
		std::uint16_t m_id{};
	};

	export class Regions final {
	  public:
		Regions();

		Region Create(const cecore::Rect2Di32& a_initial) {
			auto id = m_service.Create(a_initial);
			return Region{m_service, id};
		}

		Region GetActive() const {
			auto active = m_service.GetActive();
			if(active == RegionService::s_invalidRegion) { return Region{}; }
			return Region(m_service, active);
		}

	  private:
		RegionService& m_service;
	};
}    // namespace CR::Engine::Input

module :private;

ceinput::Regions::Regions() : m_service(cecore::GetService<ceinput::RegionService>()) {}