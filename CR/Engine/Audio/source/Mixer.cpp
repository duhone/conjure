module CR.Engine.Audio.Mixer;

import CR.Engine.Audio.MixerSystem;

namespace cea = CR::Engine::Audio;

cea::Mixer::Mixer([[maybe_unused]] uint64_t a_key) {
	// m_id = GetToneSystem().Create(a_name, a_freq);
}

void cea::Mixer::FreeHandle() {
	// GetToneSystem().Delete(m_id);
}

void cea::Mixer::SetVolume([[maybe_unused]] float a_vol) {
	// GetToneSystem().SetVolume(m_id, a_vol);
}
