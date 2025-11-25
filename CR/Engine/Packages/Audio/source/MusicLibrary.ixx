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
	std::unordered_map<uint64_t, uint16_t> m_lookup;
	std::vector<std::string> m_names;
	std::vector<std::string> m_paths;

	struct MusicDataSource {
		ma_data_source_base base{};
		uint32_t index{};
		drflac* flacHandle{};
		uint32_t sizeFrames{};
		uint64_t cursor{};
		std::span<std::byte> flacData;
	};
	MusicDataSource m_dataSource{};
	std::vector<ceasset::Handles::Asset> m_handles;

	int32_t m_current{-1};
	int32_t m_requested{-1};

	ma_result DataSourceRead(ma_data_source* maDataSource, void* pFramesOut, ma_uint64 frameCount,
	                         ma_uint64* pFramesRead) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");
		CR_ASSERT(maDataSource == &m_dataSource, "Unexpected music data source");

		uint64_t framesRead{};
		if(pFramesOut) {
			framesRead =
			    drflac_read_pcm_frames_s16(m_dataSource.flacHandle, frameCount, (int16_t*)pFramesOut);
		}

		m_dataSource.cursor += framesRead;

		if(framesRead == 0) { return MA_AT_END; }

		if(pFramesRead) { *pFramesRead = framesRead; }

		return MA_SUCCESS;
	}

	ma_result DataSourceSeek(ma_data_source* maDataSource, ma_uint64 frameIndex) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");
		CR_ASSERT(maDataSource == &m_dataSource, "Unexpected music data source");

		if(drflac_seek_to_pcm_frame(m_dataSource.flacHandle, frameIndex) == DRFLAC_FALSE) { return MA_ERROR; }

		m_dataSource.cursor = frameIndex;

		return MA_SUCCESS;
	}

	ma_result DataSourceGetDataFormat(ma_data_source* maDataSource, ma_format* pFormat, ma_uint32* pChannels,
	                                  ma_uint32* pSampleRate, ma_channel* pChannelMap,
	                                  [[maybe_unused]] size_t channelMapCap) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");
		CR_ASSERT(maDataSource == &m_dataSource, "Unexpected music data source");

		if(pFormat) { *pFormat = ma_format_s16; }
		if(pChannels) { *pChannels = 1; }
		if(pSampleRate) { *pSampleRate = CR::Engine::Audio::Constants::c_sampleRate; }
		if(pChannelMap) { *pChannelMap = MA_CHANNEL_MONO; }

		return MA_SUCCESS;
	}

	ma_result DataSourceGetCursor(ma_data_source* maDataSource, ma_uint64* pCursor) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");
		CR_ASSERT(maDataSource == &m_dataSource, "Unexpected music data source");

		if(pCursor) *pCursor = m_dataSource.cursor;

		return MA_SUCCESS;
	}

	ma_result DataSourceGetLength(ma_data_source* maDataSource, ma_uint64* pLength) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");
		CR_ASSERT(maDataSource == &m_dataSource, "Unexpected music data source");

		if(pLength) *pLength = m_dataSource.sizeFrames;

		return MA_SUCCESS;
	}

	void DataSourceInit(MusicDataSource& a_dataSource) {
		static ma_data_source_vtable dataSourceVtable = {DataSourceRead, DataSourceSeek,
		                                                 DataSourceGetDataFormat, DataSourceGetCursor,
		                                                 DataSourceGetLength};

		ma_result result;
		ma_data_source_config baseConfig;

		baseConfig        = ma_data_source_config_init();
		baseConfig.vtable = &dataSourceVtable;

		result = ma_data_source_init(&baseConfig, &a_dataSource.base);
		CR_ASSERT(result == MA_SUCCESS, "failed to initialize data source");
	}
}    // namespace

void cea::MusicLibrary::Initialize() {
	flatbuffers::Parser parser = ceasset::GetData(cecore::C_Hash64("Audio/music.json"), SCHEMAS_MUSIC);
	auto music                 = Flatbuffers::GetMusic(parser.builder_.GetBufferPointer());
	for(const auto& song : *music->music()) {
		m_lookup[cecore::Hash64(song->name()->c_str())] = (uint16_t)m_handles.size();
		m_names.push_back(song->name()->c_str());
		m_paths.push_back(song->path()->c_str());

		auto songHandle = ceasset::GetHandle(cecore::Hash64(m_paths.back()));
		m_handles.push_back(songHandle);

		// ceasset::Open(songHandle);
		// auto songData    = ceasset::GetData(songHandle);
		// auto& dataSource = m_flacDatas.emplace_back();
		// dataSource       = songData;
		// ceasset::Close(songHandle);
	}

	DataSourceInit(m_dataSource);
}

void cea::MusicLibrary::Shutdown() {
	ma_data_source_uninit(&m_dataSource.base);
}

uint16_t cea::MusicLibrary::GetIndex(uint64_t a_nameHash) {
	auto iter = m_lookup.find(a_nameHash);
	CR_ASSERT(iter != m_lookup.end(), "Could not find audio music asset {}", a_nameHash);
	return iter->second;
}
