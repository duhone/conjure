export module CR.Engine.Audio.Music;

import CR.Engine.Core;

import CR.Engine.Audio.MusicLibrary;

import <string_view>;

namespace CR::Engine::Audio {
	export class HandleMusic {
		friend HandleMusic GetHandleMusic();

	  public:
		void Play(uint64_t a_nameHash);

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