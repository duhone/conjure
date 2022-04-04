export module CR.Engine.Audio.Constants;

import<cstdint>;

namespace CR::Engine::Audio {
	constexpr inline uint32_t c_mixChannels       = 2;
	constexpr inline uint32_t c_mixSampleRate     = 48000;
	constexpr inline uint32_t c_mixBytesPerSample = 4;    // float
}    // namespace CR::Engine::Audio
