export module CR.Engine.Audio.FX;

import CR.Engine.Core;

import CR.Engine.Audio.FXLibrary;

import <string_view>;

namespace CR::Engine::Audio {
	export class HandleFXs {
		friend HandleFXs GetHandleFXs();

	  public:
		void SetVolume(float a_volume);

	  private:
		HandleFXs(FXLibrary& a_library) : m_library(a_library) {}

		FXLibrary& m_library;
	};

	export class HandleFX {
		friend HandleFX GetHandleFX(uint64_t);

	  public:
		void Play();

	  private:
		HandleFX(FXLibrary& a_library, uint16_t a_index) : m_library(a_library), m_index(a_index) {}

		FXLibrary& m_library;
		uint16_t m_index;
	};

	export [[nodiscard]] HandleFXs GetHandleFXs();
	export [[nodiscard]] HandleFX GetHandleFX(uint64_t a_fxPathHash);
}    // namespace CR::Engine::Audio

module :private;

namespace cecore = CR::Engine::Core;
namespace ceaud  = CR::Engine::Audio;

ceaud::HandleFXs ceaud::GetHandleFXs() {
	auto& library = cecore::GetService<FXLibrary>();

	return {library};
}

ceaud::HandleFX ceaud::GetHandleFX(uint64_t a_fxPathHash) {
	auto& library = cecore::GetService<FXLibrary>();
	auto index    = library.GetIndex(a_fxPathHash);

	return {library, index};
}

void ceaud::HandleFX::Play() {
	m_library.Play(m_index);
}

void ceaud::HandleFXs::SetVolume(float a_volume) {
	m_library.SetVolume(a_volume);
}