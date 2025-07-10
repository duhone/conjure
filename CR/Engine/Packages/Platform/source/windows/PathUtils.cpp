module;

#include <platform/windows/CRWindows.h>

module CR.Engine.Platform.PathUtils;

namespace fs = std::filesystem;

std::filesystem::path CR::Engine::Platform::GetCurrentProcessPath() {
	wchar_t filenameWithPath[4096];    // 4096 should be enough for anyone
	GetModuleFileNameW(nullptr, filenameWithPath, sizeof(filenameWithPath));
	fs::path pathOnly{filenameWithPath};
	pathOnly = pathOnly.parent_path();
	return pathOnly;
}
