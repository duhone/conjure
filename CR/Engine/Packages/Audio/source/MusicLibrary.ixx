/*module;

#include "generated/audio/music_generated.h"

#include "flatbuffers/idl.h"

#include "core/Log.h"

#include <dr_flac.h>

export module CR.Engine.Audio.MusicLibrary;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import CR.Engine.Audio.Constants;
import CR.Engine.Audio.Sample;
import CR.Engine.Audio.Utilities;

import <filesystem>;
import <typeindex>;
import <span>;
import <string>;
import <vector>;
import <unordered_map>;

namespace CR::Engine::Audio {
    export class MusicLibrary {
      public:
        static inline constexpr uint64_t s_typeIndex = CR::Engine::Core::EightCC("EAudMusi");

        MusicLibrary();

        void Stop();

        void Play(uint64_t a_nameHash) {
            uint16_t index = GetIndex(a_nameHash);
            CR_ASSERT(index < m_flacData.size(), "Trying to play an invalid Music {}", index);
            m_playRequest([index](std::optional<uint16_t>& a_request) { a_request.emplace(index); });
        }

        void Mix(std::span<Sample> a_data);

        float GetVolume() const { return CR::Engine::Audio::CalcLinearVolume(m_volume); }
        void SetVolume(float a_volume) { m_volume = CR::Engine::Audio::CalcLogVolume(a_volume); }
        bool GetMute() const { return m_mute; }
        void SetMute(bool a_mute) { m_mute = a_mute; }

      private:
        // quarter second
        constexpr inline static uint32_t c_transitionSamples = CR::Engine::Audio::c_mixSampleRate / 4;
        constexpr inline static float c_fadeStep             = 1.0f / c_transitionSamples;

        uint16_t GetIndex(uint64_t a_nameHash) const noexcept {
            auto iter = m_lookup.find(a_nameHash);
            CR_ASSERT(iter != m_lookup.end(), "Could not find audio music asset {}", a_nameHash);
            return iter->second;
        }

        std::unordered_map<uint64_t, uint16_t> m_lookup;
        std::vector<std::string> m_names;
        std::vector<std::string> m_paths;
        std::vector<std::vector<std::byte>> m_flacData;

        CR::Engine::Core::Locked<std::optional<uint16_t>> m_playRequest;

        struct Playing {
            uint16_t Index;
            drflac* FlacPtr{};
        };
        std::optional<Playing> m_playing;
        std::optional<uint16_t> m_pending;
        uint32_t m_transition{0};
        float m_volume{1.0f};
        bool m_mute{};
    };
}    // namespace CR::Engine::Audio

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace ceplat  = CR::Engine::Platform;
namespace cea     = CR::Engine::Audio;

namespace fs = std::filesystem;

cea::MusicLibrary::MusicLibrary() {
    auto& assetService = cecore::GetService<ceasset::Service>();

    flatbuffers::Parser parser = assetService.GetData(cecore::C_Hash64("Audio/music.json"), SCHEMAS_MUSIC);
    auto music                 = Flatbuffers::GetMusic(parser.builder_.GetBufferPointer());
    for(const auto& song : *music->music()) {
        m_lookup[cecore::Hash64(song->name()->c_str())] = (uint16_t)m_flacData.size();
        m_names.push_back(song->name()->c_str());
        m_paths.push_back(song->path()->c_str());

        auto songHandle = assetService.GetHandle(cecore::Hash64(m_paths.back()));
        assetService.Open(songHandle);
        auto songData = assetService.GetData(songHandle);
        m_flacData.emplace_back(songData.begin(), songData.end());
        assetService.Close(songHandle);
    }
}

void cea::MusicLibrary::Stop() {
    if(m_playing.has_value()) { drflac_close(m_playing.value().FlacPtr); }
}

void cea::MusicLibrary::Mix(std::span<Sample> a_data) {
    m_playRequest([this](std::optional<uint16_t>& a_request) {
        if(a_request.has_value()) {
            if(m_playing.has_value()) {
                m_pending    = a_request;
                m_transition = c_transitionSamples;
            } else {
                CR_ASSERT_AUDIT(!m_playing.has_value(), "should be no need to free drflac here");
                const auto& flacData = m_flacData[a_request.value()];
                auto drFlac          = drflac_open_memory(flacData.data(), flacData.size(), nullptr);
                m_playing.emplace(a_request.value(), drFlac);
            }
            a_request.reset();
        }
    });

    if(m_pending.has_value() && m_transition <= 0) {
        if(m_playing.has_value()) { drflac_close(m_playing.value().FlacPtr); }
        const auto& flacData = m_flacData[m_pending.value()];
        auto drFlac          = drflac_open_memory(flacData.data(), flacData.size(), nullptr);
        m_playing.emplace(m_pending.value(), drFlac);
        m_pending.reset();
    }

    if(!m_playing.has_value() && !m_pending.has_value()) { return; }
    CR_ASSERT(!(!m_playing.has_value() && m_pending.has_value()),
              "Should not be possible to have a pending, but not playing music track");

    float fade = (float)m_transition / c_transitionSamples;
    CR_ASSERT(!m_pending.has_value() || (fade > 0.0f && fade <= 1.0f), "unexpeded fade value");

    float* pcmData  = (float*)alloca(a_data.size() * sizeof(float));
    auto framesRead = drflac_read_pcm_frames_f32(m_playing.value().FlacPtr, a_data.size(), pcmData);
    while(framesRead < a_data.size()) {
        drflac_seek_to_pcm_frame(m_playing.value().FlacPtr, 0);
        framesRead += drflac_read_pcm_frames_f32(m_playing.value().FlacPtr, a_data.size() - framesRead,
                                                 pcmData + framesRead);
    }

    for(int32_t i = 0; i < a_data.size(); ++i) {
        float sample = pcmData[i];
        if(m_pending.has_value()) {
            sample *= fade;
            fade = std::max(0.0f, fade - c_fadeStep);
        }
        sample *= m_volume;
        if(m_mute) { sample = 0.0f; }
        a_data[i].Left += sample;
        a_data[i].Right += sample;
    }
}
*/