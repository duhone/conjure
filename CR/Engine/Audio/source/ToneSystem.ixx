export module CR.Engine.Audio.ToneSystem;

import CR.Engine.Audio.Constants;
import CR.Engine.Audio.Sample;

import CR.Engine.Core;

import<span>;

namespace CR::Engine::Audio {
	export class ToneSystem {
	  public:
		ToneSystem()                  = default;
		~ToneSystem()                 = default;
		ToneSystem(const ToneSystem&) = delete;
		ToneSystem(ToneSystem&&)      = delete;
		ToneSystem& operator=(const ToneSystem&) = delete;
		ToneSystem& operator=(ToneSystem&&) = delete;

		void Mix(std::span<Sample> a_data);

	  private:
		struct ToneData {
			float Frequency{0.0f};
			float volume{1.0f};
		};

		Core::Table<64, std::string, ToneData> m_toneTable;
	};
}    // namespace CR::Engine::Audio

module : private;

namespace cea = CR::Engine::Audio;

void cea::ToneSystem::Mix([[maybe_unused]] std::span<Sample> a_data) {}