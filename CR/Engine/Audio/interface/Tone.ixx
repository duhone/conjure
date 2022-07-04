export module CR.Engine.Audio.Tone;

import CR.Engine.Core;

import<string_view>;

namespace CR::Engine::Audio {
	export class Tone : public CR::Engine::Core::Handle<Tone> {
		friend CR::Engine::Core::Handle<Tone>;

	  public:
		Tone() = default;
		Tone(std::string_view a_name, float a_freq);
		~Tone()                       = default;
		Tone(const Tone&)             = delete;
		Tone(Tone&& a_other) noexcept = default;
		Tone& operator=(const Tone&) = delete;
		Tone& operator=(Tone&& a_other) noexcept = default;

		void SetFrequency(float a_freq);
		void SetVolume(float a_vol);

	  private:
		void FreeHandle();

		static class ToneSystem* s_toneSystem;
	};
}    // namespace CR::Engine::Audio