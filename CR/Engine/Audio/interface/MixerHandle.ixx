export module CR.Engine.Audio.MixerHandle;

import<cinttypes>;
import<utility>;

namespace CR::Engine::Audio {
	class MixerSystem;
	class Mixer;

	export class MixerHandle {
		friend MixerSystem;

	  public:
		// A default constructed Mixer isn't usable, must obtain a usable one by grabbing the master mixer, or
		// creating a mixer from another one(like the master)
		MixerHandle() = default;
		~MixerHandle();

		MixerHandle(const MixerHandle&) = delete;
		MixerHandle& operator=(const MixerHandle&) = delete;
		MixerHandle(MixerHandle&& a_other) noexcept { *this = std::move(a_other); }
		MixerHandle& operator=(MixerHandle&& a_other) noexcept;

	  private:
		constexpr static int32_t c_unused{-1};

		MixerHandle(MixerSystem* a_system, Mixer* a_mixer) : m_system(a_system), m_mixer(a_mixer) {}

		int32_t m_id{c_unused};
		MixerSystem* m_system{nullptr};
		Mixer* m_mixer{nullptr};
	};
}    // namespace CR::Engine::Audio
