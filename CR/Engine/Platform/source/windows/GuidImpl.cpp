#include "GuidImpl.h"

#pragma warning(push)
#pragma warning(disable : 5105)
#include <objbase.h>
#pragma warning(pop)

CR::Engine::Core::Guid CreateGuidWin() {
	GUID guid;
	if(CoCreateGuid(&guid) != S_OK) { return CR::Engine::Core::Guid::Null(); }
	return CR::Engine::Core::Guid{(uint32_t)guid.Data1, *(uint32_t*)(&guid.Data2), *(uint32_t*)guid.Data4,
	                              *((uint32_t*)&guid.Data4[4])};
}