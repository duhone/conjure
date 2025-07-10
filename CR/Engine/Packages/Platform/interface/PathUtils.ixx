export module CR.Engine.Platform.PathUtils;

import<filesystem>;

export namespace CR::Engine::Platform {
	std::filesystem::path GetCurrentProcessPath();
}    // namespace CR::Engine::Platform