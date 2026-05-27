module;

#include <core/Core.h>

#include <miniaudio.h>

module CR.Engine.Audio;

import CR.Engine.Core;
import CR.Engine.Assets;

import CR.Engine.Audio.Music;
import CR.Engine.Audio.SoundFX;

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
	ma_sound_group_init_ex(&m_minAudio, &fxGroupConfig, &m_fx);
	ma_node_attach_output_bus(&m_fx, 0, ma_engine_get_endpoint(&m_minAudio), 0);

	ceaudio::SoundFX::Initialize(m_minAudio, m_fx);
	ceaudio::Music::Initialize(m_minAudio, m_music);
}

void ceaudio::Update() {
	if(!m_enabled) { return; }
	ceaudio::SoundFX::Update();
	ceaudio::Music::Update();
}

void ceaudio::Shutdown() {
	if(!m_enabled) { return; }

	ceaudio::SoundFX::Shutdown();
	ceaudio::Music::Shutdown();

	ma_sound_group_uninit(&m_music);
	ma_sound_group_uninit(&m_fx);
	ma_engine_uninit(&m_minAudio);
}

void ceaudio::setFXVolume(float a_volume) {
	ma_sound_group_set_volume(&m_fx, a_volume);
}

void ceaudio::setMusicVolume(float a_volume) {
	ma_sound_group_set_volume(&m_music, a_volume);
}

[[nodiscard]] ceaudio::Handles::SoundFX ceaudio::SoundFX::GetHandle(uint64_t a_nameHash) {
	return SoundFX::GetHandleImpl(a_nameHash);
}

void ceaudio::SoundFX::Play(Handles::SoundFX a_handle) {
	SoundFX::PlayImpl(a_handle);
}

[[nodiscard]] ceaudio::Handles::Music ceaudio::Music::GetHandle([[maybe_unused]] uint64_t a_nameHash) {
	return Music::GetHandleImpl(a_nameHash);
}

void ceaudio::Music::Play([[maybe_unused]] Handles::Music a_handle) {
	Music::PlayImpl(a_handle);
}

void ceaudio::Music::Stop() {
	Music::Stop();
}