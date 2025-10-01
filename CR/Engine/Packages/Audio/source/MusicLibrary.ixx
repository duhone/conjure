module;

#include "generated/audio/music_generated.h"

#include "flatbuffers/idl.h"

#include "core/Log.h"

#include <dr_flac.h>
#include <miniaudio.h>

export module CR.Engine.Audio.MusicLibrary;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import CR.Engine.Audio.Constants;

import std;
import std.compat;

namespace CR::Engine::Audio::MusicLibrary {

	void Initialize();
	void Shutdown();

	uint16_t GetIndex(uint64_t a_nameHash);
}    // namespace CR::Engine::Audio::MusicLibrary

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace ceplat  = CR::Engine::Platform;
namespace cea     = CR::Engine::Audio;

namespace fs = std::filesystem;

namespace {
	enum class State { Idle, IdleToPlay, Playing, PlayingToFade, FadingToPlay, FadingToIdle };
	std::unordered_map<uint64_t, uint16_t> m_lookup;
	std::vector<std::string> m_names;
	std::vector<std::string> m_paths;

	struct MusicDataSource {
		ma_data_source_base base{};
		uint32_t index{};
	};
	MusicDataSource m_dataSource{};
	std::vector<CR::Engine::Core::Buffer> m_flacDatas;

	State m_state;
}    // namespace

void cea::MusicLibrary::Initialize() {
	flatbuffers::Parser parser = ceasset::GetData(cecore::C_Hash64("Audio/music.json"), SCHEMAS_MUSIC);
	auto music                 = Flatbuffers::GetMusic(parser.builder_.GetBufferPointer());
	for(const auto& song : *music->music()) {
		m_lookup[cecore::Hash64(song->name()->c_str())] = (uint16_t)m_flacDatas.size();
		m_names.push_back(song->name()->c_str());
		m_paths.push_back(song->path()->c_str());

		auto songHandle = ceasset::GetHandle(cecore::Hash64(m_paths.back()));
		ceasset::Open(songHandle);
		auto songData    = ceasset::GetData(songHandle);
		auto& dataSource = m_flacDatas.emplace_back();
		dataSource       = songData;
		ceasset::Close(songHandle);
	}
}

void cea::MusicLibrary::Shutdown() {}

uint16_t cea::MusicLibrary::GetIndex(uint64_t a_nameHash) {
	auto iter = m_lookup.find(a_nameHash);
	CR_ASSERT(iter != m_lookup.end(), "Could not find audio music asset {}", a_nameHash);
	return iter->second;
}
