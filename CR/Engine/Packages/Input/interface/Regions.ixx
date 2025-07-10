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
		Region()  = default;
		~Region() = default;

		bool operator==(const Region& a_other) const = default;
		bool operator!=(const Region& a_other) const = default;

		bool IsValid() const { return m_id != RegionService::s_invalidRegion; }

	  private:
		Region(std::uint16_t a_id) : m_id(a_id) {}

		std::uint16_t m_id{RegionService::s_invalidRegion};
	};

	export class Regions final {
	  public:
		Regions();

		Region Create(const cecore::Rect2Di32& a_initial) {
			auto id = m_service.Create(a_initial);
			return Region{id};
		}

		void Delete(Region& a_region) {
			CR_ASSERT_AUDIT(a_region.IsValid(), "trying to delete an invalid region");
			m_service.Delete(a_region.m_id);
			a_region = Region{};
		}

		void Update(Region a_region, const cecore::Rect2Di32& a_value) {
			CR_ASSERT_AUDIT(a_region.IsValid(), "trying to update an invalid region");
			m_service.Update(a_region.m_id, a_value);
		}

		Region GetActive() const { return Region(m_service.GetActive()); }

		bool WasActiveClicked() const { return (m_service.GetCursorState() & CursorStates::Clicked) != 0; }

	  private:
		RegionService& m_service;
	};
}    // namespace CR::Engine::Input

module :private;

ceinput::Regions::Regions() : m_service(cecore::GetService<ceinput::RegionService>()) {}