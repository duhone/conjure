module;

#include "core/Log.h"

#include <glm/glm.hpp>

export module CR.Engine.Input.RegionService;

import CR.Engine.Core;

import <array>;
import <bitset>;
import <typeindex>;

namespace cecore = CR::Engine::Core;

using namespace cecore::Literals;

namespace CR::Engine::Input {
	export class RegionService {
		static inline const std::uint32_t c_maxRegions = 512;
		static_assert(c_maxRegions < 64_KB);

	  public:
		static inline constexpr uint64_t s_typeIndex     = cecore::EightCC("EInpRegn");
		static inline constexpr uint16_t s_invalidRegion = std::numeric_limits<std::uint16_t>::max();

		std::uint16_t Create(const cecore::Rect2Di32& a_initial);
		const cecore::Rect2Di32& Retrieve(std::uint16_t a_id) const;
		cecore::Rect2Di32& Retrieve(std::uint16_t a_id);
		void Update(std::uint16_t a_id, const cecore::Rect2Di32& a_region);
		void Delete(std::uint16_t a_id);

		void UpdateCursor(bool a_enabled, const glm::ivec2& a_position);

		void Update();

		std::uint16_t GetActive() const { return m_activeRegion; }

	  private:
		cecore::BitSet<c_maxRegions> m_regionsUsed;
		std::array<cecore::Rect2Di32, c_maxRegions> m_regions;
		bool m_cursorEnabled{};
		glm::ivec2 m_cursorPosition;
		std::uint16_t m_activeRegion;
	};
}    // namespace CR::Engine::Input

module :private;

namespace ceinput = CR::Engine::Input;

std::uint16_t ceinput::RegionService::Create([[maybe_unused]] const cecore::Rect2Di32& a_initial) {
	auto avail = m_regionsUsed.FindNotInSet();
	m_regionsUsed.insert(avail);
	m_regions[avail] = a_initial;
	return avail;
}

const cecore::Rect2Di32& ceinput::RegionService::Retrieve(std::uint16_t a_id) const {
	CR_ASSERT_AUDIT(a_id < c_maxRegions, "invalid input region id");
	return m_regions[a_id];
}

cecore::Rect2Di32& ceinput::RegionService::Retrieve(std::uint16_t a_id) {
	CR_ASSERT_AUDIT(a_id < c_maxRegions, "invalid input region id");
	return m_regions[a_id];
}

void ceinput::RegionService::Update(std::uint16_t a_id, const cecore::Rect2Di32& a_region) {
	CR_ASSERT_AUDIT(a_id < c_maxRegions, "invalid input region id");
	m_regions[a_id] = a_region;
}

void ceinput::RegionService::Delete(std::uint16_t a_id) {
	CR_ASSERT_AUDIT(a_id < c_maxRegions, "invalid input region id");
	CR_ASSERT_AUDIT(m_regionsUsed.contains(a_id), "id {} to delete doesn't exist", a_id);
	m_regionsUsed.erase(a_id);
}

void ceinput::RegionService::UpdateCursor(bool a_enabled, const glm::ivec2& a_position) {
	m_cursorEnabled  = a_enabled;
	m_cursorPosition = a_position;
}

void ceinput::RegionService::Update() {
	if(!m_cursorEnabled) { return; }

	m_activeRegion = s_invalidRegion;
	for(const auto& region : m_regionsUsed) {
		if(m_regions[region].Contains(m_cursorPosition)) { m_activeRegion = region; }
	}
}