module;

#include <core/Core.h>

#include <miniaudio.h>

export module CR.Engine.Audio;

export import CR.Engine.Audio.Handles;

import std;
import std.compat;

export namespace CR::Engine::Audio {
	void Initialize();
	void Update();
	void Shutdown();

	void setFXVolume(float a_volume);
	void setMusicVolume(float a_volume);

	namespace SoundFX {
		// sound fx handles are valid for life of audio engine
		extern "C++" [[nodiscard]] Handles::SoundFX GetHandle(uint64_t a_nameHash);
		// soundfx play once then stop. every call to play for a given handle starts playing a new instance of
		// that sound fx.
		extern "C++" void Play(Handles::SoundFX a_handle);
	}    // namespace SoundFX

	namespace Music {
		// music handles are valid for life of audio engine
		extern "C++" [[nodiscard]] Handles::Music GetHandle(uint64_t a_nameHash);
		// Only one music will play at a time, this call replaces the currently playing music.
		extern "C++" void Play(Handles::Music a_handle);
		// Stops currently playing music, a no-op if no music is playing
		extern "C++" void Stop();
	}    // namespace Music
}    // namespace CR::Engine::Audio
