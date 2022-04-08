export module CR.Engine.Audio.MixerSystem;

import CR.Engine.Audio.Mixer;
import CR.Engine.Audio.MixerHandle;

import<bitset>;

namespace CR::Engine::Audio {
	export class MixerSystem {
	  public:
		MixerSystem()  = default;
		~MixerSystem() = default;

		MixerSystem(const MixerSystem&)    = delete;
		MixerSystem(MixerSystem&& a_other) = delete;

		MixerSystem& operator=(const MixerSystem&) = delete;
		MixerSystem& operator=(MixerSystem&&) = delete;

		[[nodiscard]] MixerHandle CreateMixer();

	  private:
		constexpr static int32_t c_maxMixers = 16;

		std::bitset<c_maxMixers> m_used;
		Mixer m_mixers[c_maxMixers];
	};
}    // namespace CR::Engine::Audio

module : private;

namespace cea = CR::Engine::Audio;

[[nodiscard]] cea::MixerHandle cea::MixerSystem::CreateMixer() {
	return MixerHandle{};
}

// Handle class

cea::MixerHandle::~MixerHandle() {}

cea::MixerHandle& cea::MixerHandle::operator=(cea::MixerHandle&& a_other) noexcept {
	if(m_id >= 0) { this->~MixerHandle(); }

	m_id     = a_other.m_id;
	m_system = a_other.m_system;

	a_other.m_id = c_unused;

	return *this;
}
