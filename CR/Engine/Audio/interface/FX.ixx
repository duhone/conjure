export module CR.Engine.Audio.FX;

import CR.Engine.Audio.Services;
import CR.Engine.Audio.FXLibrary;

import <string_view>;

namespace CR::Engine::Audio {
	export struct HandleFX {
		FXLibrary& Library;
		uint16_t Index;
	};

	export [[nodiscard]] HandleFX GetHandle(std::string_view a_fxPath);
	export void PlayFX(HandleFX handle);
}    // namespace CR::Engine::Audio

module :private;

namespace ceaud = CR::Engine::Audio;

ceaud::HandleFX ceaud::GetHandle(std::string_view a_fxPath) {
	auto& library = GetService<FXLibrary>();
	auto index    = library.GetIndex(a_fxPath);

	return {library, index};
}

void ceaud::PlayFX(HandleFX handle) {
	handle.Library.Play(handle.Index);
}