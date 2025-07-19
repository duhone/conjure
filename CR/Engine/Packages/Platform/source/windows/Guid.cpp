module;

#include "CRWindows.h"

module CR.Engine.Platform.Guid;

namespace cecore = CR::Engine::Core;

cecore::Guid CR::Engine::Platform::CreateGuid() {
	GUID guid;
	if(CoCreateGuid(&guid) != S_OK) { return CR::Engine::Core::Guid::Null(); }
	return CR::Engine::Core::Guid{(uint32_t)guid.Data1, *(uint32_t*)(&guid.Data2), *(uint32_t*)guid.Data4,
	                              *((uint32_t*)&guid.Data4[4])};
}
