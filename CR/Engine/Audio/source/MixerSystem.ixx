export module CR.Engine.Audio.MixerSystem;

import CR.Engine.Audio.Utilities;

import CR.Engine.Core;

import<bitset>;
import<cstdint>;
import<shared_mutex>;

namespace CR::Engine::Audio {
	export class MixerSystem {
	  public:
		MixerSystem()  = default;
		~MixerSystem() = default;

		MixerSystem(const MixerSystem&)    = delete;
		MixerSystem(MixerSystem&& a_other) = delete;

		MixerSystem& operator=(const MixerSystem&) = delete;
		MixerSystem& operator=(MixerSystem&&) = delete;

		uint16_t Create(uint64_t a_key);
		uint16_t Create(uint64_t a_key, uint16_t a_parentIdx);
		void Delete(uint16_t a_idx);

		// gets the combined volume of this mixer and all parent mixers
		float GetVolume(uint16_t a_idx);
		// set volume on given mixer
		void SetVolume(uint16_t a_idx, float a_vol);

	  private:
		struct MixerData {
			uint16_t Parent;
			float Volume;
		};

		std::mutex m_mutex;
		Core::Table<64, uint64_t, MixerData> m_mixerTable{"Mixer System Table"};
	};
}    // namespace CR::Engine::Audio

namespace cea = CR::Engine::Audio;

inline float cea::MixerSystem::GetVolume(uint16_t a_idx) {
	float result = 1.0f;

	while(a_idx != m_mixerTable.c_unused) {
		const auto& data = m_mixerTable.GetValue<MixerData>(a_idx);
		result *= data.Volume;
		a_idx = data.Parent;
	}

	return result;
}

inline void cea::MixerSystem::SetVolume(uint16_t a_idx, float a_vol) {
	auto [mixer] = m_mixerTable[a_idx];
	mixer.Volume = CalcVolume(a_vol);
}

module : private;

uint16_t cea::MixerSystem::Create(uint64_t a_key) {
	return Create(a_key, m_mixerTable.c_unused);
}

uint16_t cea::MixerSystem::Create(uint64_t a_key, uint16_t a_parentIdx) {
	std::scoped_lock lock(m_mutex);
	uint16_t index = m_mixerTable.insert(a_key);
	auto [mixer]   = m_mixerTable[index];
	mixer.Volume   = 1.0f;
	mixer.Parent   = a_parentIdx;

	return index;
}

void cea::MixerSystem::Delete(uint16_t a_idx) {
	std::scoped_lock lock(m_mutex);
	m_mixerTable.erase(a_idx);
}