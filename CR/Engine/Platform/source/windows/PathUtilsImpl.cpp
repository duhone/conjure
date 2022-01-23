#include "PathUtilsImpl.h"

#include <windows.h>

namespace fs = std::filesystem;

fs::path GetCurrentProcessPathWin() {
	wchar_t filenameWithPath[4096];    // 4096 should be enough for anyone
	GetModuleFileNameW(nullptr, filenameWithPath, sizeof(filenameWithPath));
	fs::path pathOnly{filenameWithPath};
	pathOnly = pathOnly.parent_path();
	return pathOnly;
}