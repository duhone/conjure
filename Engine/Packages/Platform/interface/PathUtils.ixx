export module CR.Engine.Platform.PathUtils;

import std;

export namespace CR::Engine::Platform {
	[[nodiscard]] std::filesystem::path GetCurrentProcessPath();
}    // namespace CR::Engine::Platform