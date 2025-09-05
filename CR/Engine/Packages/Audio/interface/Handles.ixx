export module CR.Engine.Audio.Handles;

import CR.Engine.Core;

export namespace CR::Engine::Audio::Handles {
	export using Music   = CR::Engine::Core::Handle<class MusicTag>;
	export using SoundFX = CR::Engine::Core::Handle<class SoundFXTag>;
}    // namespace CR::Engine::Audio::Handles
