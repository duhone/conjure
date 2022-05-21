export module CR.Engine.Audio.Mixer;

import CR.Engine.Core;

import<cinttypes>;
import<utility>;

namespace CR::Engine::Audio {
	export class Mixer : public CR::Engine::Core::Handle<Mixer> {
		friend CR::Engine::Core::Handle<Mixer>;

	  public:
		Mixer() = default;
		Mixer(uint64_t a_key);
		Mixer(uint64_t a_key, const Mixer& a_parent);
		~Mixer() = default;

		Mixer(const Mixer&) = delete;
		Mixer& operator=(const Mixer&) = delete;
		Mixer(Mixer&& a_other) noexcept { *this = std::move(a_other); }
		Mixer& operator=(Mixer&& a_other) noexcept = default;

		void SetVolume(float a_vol);

	  private:
		void FreeHandle();
	};
}    // namespace CR::Engine::Audio
