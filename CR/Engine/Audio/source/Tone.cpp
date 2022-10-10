module CR.Engine.Audio.Tone;

import CR.Engine.Core;

import CR.Engine.Audio.ToneSystem;

namespace cecore = CR::Engine::Core;
namespace cea    = CR::Engine::Audio;

cea::ToneSystem* cea::Tone::s_toneSystem{nullptr};

cea::Tone::Tone([[maybe_unused]] std::string_view a_name, [[maybe_unused]] float a_freq) {
	if(!s_toneSystem) { s_toneSystem = &cecore::GetService<ToneSystem>(); }
	m_id = s_toneSystem->Create(a_name, a_freq);
}

void cea::Tone::FreeHandle() {
	s_toneSystem->Delete(m_id);
}

void cea::Tone::SetFrequency([[maybe_unused]] float a_freq) {
	s_toneSystem->SetFrequency(m_id, a_freq);
}

void cea::Tone::SetVolume([[maybe_unused]] float a_vol) {
	s_toneSystem->SetVolume(m_id, a_vol);
}
