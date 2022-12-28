module;

#include "core/Log.h"

export module CR.Engine.Input.RegionService;

import CR.Engine.Core;

import <typeindex>;

namespace CR::Engine::Input {
	export class RegionService {
	  public:
		static inline constexpr uint64_t s_typeIndex = CR::Engine::Core::EightCC("EInpRegn");
	};
}    // namespace CR::Engine::Input

module :private;

namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;
