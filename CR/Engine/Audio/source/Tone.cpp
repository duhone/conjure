module CR.Engine.Audio.Tone;

import CR.Engine.Audio.ToneSystem;

namespace cea = CR::Engine::Audio;

cea::Tone::Tone([[maybe_unused]] std::string_view a_name, [[maybe_unused]] float a_freq) {
	m_id = GetToneSystem().Create(a_name, a_freq);
}

void cea::Tone::FreeHandle() {
	GetToneSystem().Delete(m_id);
}

void cea::Tone::SetFrequency([[maybe_unused]] float a_freq) {
	GetToneSystem().SetFrequency(m_id, a_freq);
}

void cea::Tone::SetVolume([[maybe_unused]] float a_vol) {
	GetToneSystem().SetVolume(m_id, a_vol);
}
