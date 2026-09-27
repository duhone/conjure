export module CR.Engine.Input.Regions:Public;

import CR.Engine.Core;

import CR.Engine.Input.Handles;

import std;
import std.compat;

export namespace CR::Engine::Input::Regions {
	Handles::Region create(const CR::Engine::Core::Rect2D<int32_t>& a_initial);

	void release(Handles::Region a_region);

	void update(Handles::Region a_region, const CR::Engine::Core::Rect2D<int32_t>& a_value);

	namespace RegionStates {
		constexpr uint32_t Hover = 1 << 0;
		// If pressed or released the previous frame.
		constexpr uint32_t Pressed  = 1 << 1;
		constexpr uint32_t Released = 1 << 2;
		// Current state, either down, or up if this flag isn't there.
		constexpr uint32_t Down = 1 << 3;
	}    // namespace RegionStates

	void getStates(std::span<Handles::Region> a_regions, std::span<uint32_t> a_states);
	uint32_t getState(Handles::Region a_region) {
		uint32_t result;
		getStates(std::span<Handles::Region>(&a_region, 1), std::span<uint32_t>(&result, 1));
		return result;
	}
}    // namespace CR::Engine::Input::Regions
