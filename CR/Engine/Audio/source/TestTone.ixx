export module CR.Engine.Audio.TestTone;

import CR.Engine.Audio.Constants;
import CR.Engine.Audio.Sample;

import<array>;
import<cmath>;
import<cstdint>;
import<numbers>;
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

module : private;

namespace cea = CR::Engine::Audio;

std::span<cea::Sample> cea::TestTone::GetSamples(uint32_t& a_currentSample, uint32_t a_numSamples) {
	m_buffer.resize(a_numSamples);

	for(Sample& sample : m_buffer) {
		float toneSample =
		    sin((2 * std::numbers::pi_v<float> * m_frequency * a_currentSample) / (cea::c_mixSampleRate));
		++a_currentSample;
		sample.Left  = toneSample;
		sample.Right = toneSample;
	}

	return std::span<Sample>(std::data(m_buffer), std::size(m_buffer));
}