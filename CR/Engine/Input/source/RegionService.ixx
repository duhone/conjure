module;

#include "core/Log.h"

export module CR.Engine.Input.RegionService;

import <typeindex>;

namespace CR::Engine::Input {
	export class RegionService {
	  public:
		static std::type_index s_typeIndex;
	};
}    // namespace CR::Engine::Input

module :private;

namespace cecore  = CR::Engine::Core;
namespace ceinput = CR::Engine::Input;

std::type_index ceinput::RegionService::s_typeIndex{typeid(RegionService)};