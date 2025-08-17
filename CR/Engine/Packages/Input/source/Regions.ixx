module;

#include "core/Core.h"

// #include <glm/glm.hpp>

export module CR.Engine.Input.Regions;

export import :Public;

import CR.Engine.Input.Handles;
import CR.Engine.Input.Context;

import CR.Engine.Core;

import std;
import std.compat;

export namespace CR::Engine::Input::Regions {
	void update();
}    // namespace CR::Engine::Input::Regions

module :private;

namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;

using namespace cecore::Literals;

namespace {
	constexpr uint32_t c_maxRegions = 512;

	cecore::BitSet<c_maxRegions> m_regionsUsed;
	std::array<ceinput::Handles::Region, c_maxRegions> m_regionHandles;
	std::array<cecore::Rect2D<int32_t>, c_maxRegions> m_regions;
	std::array<uint32_t, c_maxRegions> m_states;
	uint32_t m_activeRegion{c_maxRegions};
}    // namespace

ceinput::Handles::Region ceinput::Regions::create(const cecore::Rect2D<int32_t>& a_initial) {
	CR_ASSERT(!m_regionsUsed.empty(), "ran out of regions");
	auto avail = m_regionsUsed.FindNotInSet();
	m_regionsUsed.insert(avail);
	m_regionHandles[avail].incGeneration();
	m_regions[avail] = a_initial;
	return m_regionHandles[avail];
}

void ceinput::Regions::update(Handles::Region a_region, const CR::Engine::Core::Rect2D<int32_t>& a_value) {
	CR_ASSERT(a_region < c_maxRegions, "invalid input region id");
	CR_ASSERT(m_regionHandles[a_region].getGeneration() == a_region.getGeneration(),
	          "trying to update an old handle");
	m_regions[a_region] = a_value;
}

void ceinput::Regions::release(Handles::Region a_region) {
	CR_ASSERT(a_region < c_maxRegions, "invalid input region id");
	CR_ASSERT(m_regionsUsed.contains(a_region), "id {} to delete doesn't exist", a_region);
	CR_ASSERT(m_regionHandles[a_region].getGeneration() == a_region.getGeneration(),
	          "trying to update an old handle");
	m_regionsUsed.erase(a_region);
}

void ceinput::Regions::update() {
	const auto& context = getContext();

	if((context.CursorState & CursorStates::Available) == 0) { return; }

	uint32_t oldActiveRegion = m_activeRegion;
	uint32_t newActiveRegion = c_maxRegions;
	for(uint32_t region : m_regionsUsed) {
		if(m_regions[region].Contains(context.CursorPos)) {
			m_activeRegion   = region;
			m_states[region] = RegionStates::Hover;
			if(context.CursorState & CursorStates::Down) { m_states[region] |= RegionStates::Down; }
			if(context.CursorState & CursorStates::Pressed) { m_states[region] |= RegionStates::Pressed; }
			if(context.CursorState & CursorStates::Released) { m_states[region] |= RegionStates::Released; }
			newActiveRegion = region;
			break;
		}
	}

	if(newActiveRegion != oldActiveRegion && oldActiveRegion != c_maxRegions) {
		m_states[oldActiveRegion] = 0;
	}
}

void ceinput::Regions::getStates(std::span<Handles::Region> a_regions, std::span<uint32_t> a_states) {
	CR_ASSERT(a_regions.size() == a_states.size(), "regions and states not the same size");

	for(uint32_t i = 0; i < a_regions.size(); ++i) {
		CR_ASSERT(m_regionsUsed.contains(a_regions[i]), "requested state for region that doesn't exist");
		a_states[i] = m_states[a_regions[i]];
	}
}
