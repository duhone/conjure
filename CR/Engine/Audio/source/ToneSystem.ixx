export module CR.Engine.Audio.ToneSystem;

import CR.Engine.Audio.Constants;
import CR.Engine.Audio.Sample;
import CR.Engine.Audio.Utilities;

import CR.Engine.Core;

import<cstdint>;
import<numbers>;
import<span>;
import<string_view>;

namespace CR::Engine::Audio {
	export class ToneSystem {
	  public:
		ToneSystem()                  = default;
		~ToneSystem()                 = default;
		ToneSystem(const ToneSystem&) = delete;
		ToneSystem(ToneSystem&&)      = delete;
		ToneSystem& operator=(const ToneSystem&) = delete;
		ToneSystem& operator=(ToneSystem&&) = delete;

		void Mix(std::span<Sample> a_data);

		uint16_t Create(std::string_view name, float frequency);
		void Delete(uint16_t a_id);

		void SetFrequency(uint16_t id, float a_freq);
		void SetVolume(uint16_t id, float a_vol);

	  private:
		struct ToneData {
			float Frequency{0.0f};
			float Volume{1.0f};
			int32_t CurrentSample{0};
			int32_t Period{0};
		};

		Core::Table<64, std::string, ToneData> m_toneTable{"Tone System Table"};
	};

	// temp. will eventually be owned by a mixer.
	export ToneSystem& GetToneSystem() {
		static ToneSystem ts;
		return ts;
	}
}    // namespace CR::Engine::Audio

module : private;

namespace cea = CR::Engine::Audio;

namespace {
	int32_t CalcPeriod(float a_freq) { return static_cast<int32_t>(cea::c_mixSampleRate / a_freq); }
}    // namespace

void cea::ToneSystem::Mix([[maybe_unused]] std::span<Sample> a_data) {
	for(Sample& sample : a_data) {
		for(auto& [tone] : m_toneTable) {
			float toneSample =
			    tone.Volume * sin((2 * std::numbers::pi_v<float> * tone.Frequency * tone.CurrentSample) /
			                      (cea::c_mixSampleRate));
			++tone.CurrentSample;
			sample.Left += toneSample;
			sample.Right += toneSample;
		}
	}
	for(auto& [tone] : m_toneTable) { tone.CurrentSample %= tone.Period; }
}

uint16_t cea::ToneSystem::Create(std::string_view a_name, float a_frequency) {
	uint16_t id    = m_toneTable.insert(a_name);
	auto [tone]    = m_toneTable[id];
	tone.Frequency = a_frequency;
	tone.Period    = CalcPeriod(a_frequency);

	return id;
}

void cea::ToneSystem::Delete(uint16_t a_id) {
	m_toneTable.erase(a_id);
}

void cea::ToneSystem::SetFrequency(uint16_t id, float a_freq) {
	auto [tone]    = m_toneTable[id];
	tone.Frequency = a_freq;
	tone.Period    = CalcPeriod(a_freq);
}

void cea::ToneSystem::SetVolume(uint16_t id, float a_vol) {
	auto [tone] = m_toneTable[id];
	tone.Volume = CalcVolume(a_vol);
}