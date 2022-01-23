module;

#include "GuidImpl.h"

module CR.Engine.Platform.Guid;

CR::Engine::Core::Guid CR::Engine::Platform::CreateGuid() {
	return CreateGuidWin();
}
