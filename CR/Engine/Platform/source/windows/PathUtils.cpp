module;

#include "PathUtilsImpl.h"

module CR.Engine.Platform.PathUtils;

std::filesystem::path CR::Engine::Platform::GetCurrentProcessPath() {
	return GetCurrentProcessPathWin();
}
