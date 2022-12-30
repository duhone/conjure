module;

#include "core/Log.h"

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
		static inline constexpr uint64_t s_typeIndex = cecore::EightCC("EInpRegn");

		std::uint16_t Create(const cecore::Rect2Di32& initial);

	  private:
		std::bitset<c_maxRegions> m_regionsUsed;
		std::array<cecore::Rect2Di32, c_maxRegions> m_regions;
	};
}    // namespace CR::Engine::Input

module :private;

namespace ceinput = CR::Engine::Input;

std::uint16_t ceinput::RegionService::Create([[maybe_unused]] const cecore::Rect2Di32& initial) {
	return 0;
}