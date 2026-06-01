export module CR.Engine.Audio.Constants;

import std;
import std.compat;

export namespace CR::Engine::Audio::Constants {
	inline constexpr uint32_t c_sampleRate        = 48000;
	inline constexpr uint32_t c_maxSoundFX        = 256;
	inline constexpr uint32_t c_maxSoundFXPlaying = 64;
	// only ever 1 music playing at a time.
	inline constexpr uint32_t c_maxMusic         = 256;
	inline constexpr uint64_t c_musicFadeOutTime = 500;    // ms
}    // namespace CR::Engine::Audio::Constants
