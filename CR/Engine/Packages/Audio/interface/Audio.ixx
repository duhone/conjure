module;

#include <core/Core.h>

#include <miniaudio.h>

export module CR.Engine.Audio;

export import CR.Engine.Audio.Handles;

import CR.Engine.Audio.FXLibrary;

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
	ma_sound_group m_music;
	ma_sound_group m_fx;
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

	ma_sound_group_config musicGroupConfig = ma_sound_group_config_init_2(&m_minAudio);
	musicGroupConfig.channelsIn            = 2;
	musicGroupConfig.channelsOut           = 0;
	ma_sound_group_init_ex(&m_minAudio, &musicGroupConfig, &m_music);
	ma_node_attach_output_bus(&m_music, 0, ma_engine_get_endpoint(&m_minAudio), 0);

	ma_sound_group_config fxGroupConfig = ma_sound_group_config_init_2(&m_minAudio);
	fxGroupConfig.channelsIn            = 1;
	fxGroupConfig.channelsOut           = 0;
	ma_sound_group_init_ex(&m_minAudio, &musicGroupConfig, &m_fx);
	ma_node_attach_output_bus(&m_fx, 0, ma_engine_get_endpoint(&m_minAudio), 0);

	ceaudio::FXLibrary::Initialize();
}

void ceaudio::Update() {
	if(!m_enabled) { return; }
}

void ceaudio::Shutdown() {
	if(!m_enabled) { return; }

	ceaudio::FXLibrary::Shutdown();

	ma_sound_group_uninit(&m_music);
	ma_sound_group_uninit(&m_fx);
	ma_engine_uninit(&m_minAudio);
}
