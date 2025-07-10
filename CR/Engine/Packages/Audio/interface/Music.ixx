export module CR.Engine.Audio.Music;

import CR.Engine.Core;

import CR.Engine.Audio.MusicLibrary;

import <string_view>;

namespace CR::Engine::Audio {
	export class HandleMusic {
		friend HandleMusic GetHandleMusic();

	  public:
		void Play(uint64_t a_nameHash);

		float GetVolume() const;
		void SetVolume(float a_volume);
		bool GetMute() const;
		void SetMute(bool a_mute);

	  private:
		HandleMusic(MusicLibrary& a_library) : m_library(a_library) {}

		MusicLibrary& m_library;
	};

	export [[nodiscard]] HandleMusic GetHandleMusic();
}    // namespace CR::Engine::Audio

module :private;

namespace cecore = CR::Engine::Core;
namespace ceaud  = CR::Engine::Audio;

ceaud::HandleMusic ceaud::GetHandleMusic() {
	auto& library = cecore::GetService<MusicLibrary>();

	return {library};
}

void ceaud::HandleMusic::Play(uint64_t a_nameHash) {
	m_library.Play(a_nameHash);
}

float ceaud::HandleMusic::GetVolume() const {
	return m_library.GetVolume();
}

void ceaud::HandleMusic::SetVolume(float a_volume) {
	m_library.SetVolume(a_volume);
}

bool ceaud::HandleMusic::GetMute() const {
	return m_library.GetMute();
}

void ceaud::HandleMusic::SetMute(bool a_mute) {
	m_library.SetMute(a_mute);
}