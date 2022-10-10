export module CR.Engine.Audio.FX;

import CR.Engine.Core;

import CR.Engine.Audio.FXLibrary;

import <string_view>;

namespace CR::Engine::Audio {
	export class HandleFX {
		friend HandleFX GetHandleFX(uint64_t);

	  public:
		void Play();

	  private:
		HandleFX(FXLibrary& a_library, uint16_t a_index) : m_library(a_library), m_index(a_index) {}

		FXLibrary& m_library;
		uint16_t m_index;
	};

	export [[nodiscard]] HandleFX GetHandleFX(uint64_t a_fxPathHash);
}    // namespace CR::Engine::Audio

module :private;

namespace cecore = CR::Engine::Core;
namespace ceaud  = CR::Engine::Audio;

ceaud::HandleFX ceaud::GetHandleFX(uint64_t a_fxPathHash) {
	auto& library = cecore::GetService<FXLibrary>();
	auto index    = library.GetIndex(a_fxPathHash);

	return {library, index};
}

void ceaud::HandleFX::Play() {
	m_library.Play(m_index);
}