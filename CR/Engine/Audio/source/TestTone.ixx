export module CR.Engine.Audio.TestTone;

import CR.Engine.Audio.Sample;

import<array>;
import<cstdint>;
import<span>;
import<vector>;

namespace CR::Engine::Audio {
	export class TestTone {
	  public:
		TestTone(float a_frequency) : m_frequency(a_frequency) {}

		[[nodiscard]] std::span<Sample> GetSamples(uint32_t& a_currentSample, uint32_t a_numSamples);

	  private:
		float m_frequency = 0.0f;
		std::vector<Sample> m_buffer;
	};
}    // namespace CR::Engine::Audio
