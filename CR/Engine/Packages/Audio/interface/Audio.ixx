module;

#include <core/Core.h>

#include <miniaudio.h>

export module CR.Engine.Audio;

export import CR.Engine.Audio.Handles;

import CR.Engine.Core;
import CR.Engine.Assets;

export namespace CR::Engine::Audio {
	void Initialize();
	void Update();
	void Shutdown();
}    // namespace CR::Engine::Audio

module :private;

namespace cecore   = CR::Engine::Core;
namespace ceassets = CR::Engine::Assets;
namespace ceaudio  = CR::Engine::Audio;

namespace {
	bool m_enabled{};
	ma_engine m_minAudio;
}    // namespace

void ceaudio::Initialize() {
	ma_engine_config engineConfig;
	engineConfig            = ma_engine_config_init();
	engineConfig.sampleRate = 48000;
	engineConfig.channels   = 2;

	ma_result result = ma_engine_init(&engineConfig, &m_minAudio);
	if(result != MA_SUCCESS) {
		CR_WARN("Failed to initialize audio");
		m_enabled = false;
	}
	m_enabled = false;
}

void ceaudio::Update() {}

void ceaudio::Shutdown() {
	if(!m_enabled) { return; }

	ma_engine_uninit(&m_minAudio);
}
