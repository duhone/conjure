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
	void initialize();
	void update();
}    // namespace CR::Engine::Input::Regions

module :private;

namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;

using namespace cecore::Literals;

namespace {
	constexpr uint32_t c_maxRegions = 512;

	cecore::HandlePool<ceinput::Handles::Region, c_maxRegions> m_handlePool;
	std::array<cecore::Rect2D<int32_t>, c_maxRegions> m_regions;
	std::array<uint32_t, c_maxRegions> m_states;
	ceinput::Handles::Region m_activeRegion{};
}    // namespace

void ceinput::Regions::initialize() {}

ceinput::Handles::Region ceinput::Regions::create(const cecore::Rect2D<int32_t>& a_initial) {
	CR_ASSERT(!m_handlePool.exhausted(), "ran out of regions");
	auto avail       = m_handlePool.aquire();
	m_regions[avail] = a_initial;
	return avail;
}

void ceinput::Regions::update(Handles::Region a_region, const CR::Engine::Core::Rect2D<int32_t>& a_value) {
	CR_ASSERT(a_region < c_maxRegions, "invalid input region id");
	CR_ASSERT(m_handlePool.isValid(a_region), "trying to update an old or invalid handle");
	m_regions[a_region] = a_value;
}

void ceinput::Regions::release(Handles::Region a_region) {
	CR_ASSERT(a_region < c_maxRegions, "invalid input region id");
	CR_ASSERT(m_handlePool.isValid(a_region), "trying to update an old or invalid handle");
	m_handlePool.release(a_region);
}

void ceinput::Regions::update() {
	const auto& context = getContext();

	if((context.CursorState & CursorStates::Available) == 0) { return; }

	Handles::Region oldActiveRegion = m_activeRegion;
	Handles::Region newActiveRegion{};
	m_handlePool.iterate([&](const auto& handle) {
		if(m_regions[handle].Contains(context.CursorPos)) {
			m_activeRegion   = handle;
			m_states[handle] = RegionStates::Hover;
			if(context.CursorState & CursorStates::Down) { m_states[handle] |= RegionStates::Down; }
			if(context.CursorState & CursorStates::Pressed) { m_states[handle] |= RegionStates::Pressed; }
			if(context.CursorState & CursorStates::Released) { m_states[handle] |= RegionStates::Released; }
			newActiveRegion = handle;
			return false;
		}

		return true;
	});

	if(newActiveRegion != oldActiveRegion && m_handlePool.isValid(oldActiveRegion)) {
		m_states[oldActiveRegion] = 0;
	}
}

void ceinput::Regions::getStates(std::span<Handles::Region> a_regions, std::span<uint32_t> a_states) {
	CR_ASSERT(a_regions.size() == a_states.size(), "regions and states not the same size");

	for(uint32_t i = 0; i < a_regions.size(); ++i) {
		CR_ASSERT(m_handlePool.isValid(a_regions[i]), "requested state for region that doesn't exist");
		a_states[i] = m_states[a_regions[i]];
	}
}
