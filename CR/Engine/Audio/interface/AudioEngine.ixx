export module CR.Engine.Audio.Engine;

import<filesystem>;

namespace CR::Engine::Audio {
	export void EngineStart(bool a_checkForClipping, std::filesystem::path a_fxFolder,
	                        std::filesystem::path a_musicFolder);
	export void EngineStop();
}    // namespace CR::Engine::Audio
