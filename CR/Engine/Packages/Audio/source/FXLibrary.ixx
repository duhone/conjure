module;

#include "generated/audio/soundfx_generated.h"

#include "flatbuffers/idl.h"

#include "core/Log.h"

#include <dr_flac.h>
#include <miniaudio.h>

export module CR.Engine.Audio.FXLibrary;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import CR.Engine.Audio.Constants;
import CR.Engine.Audio.Utilities;

import std;
import std.compat;

export namespace CR::Engine::Audio::FXLibrary {
	void Initialize();
	void Shutdown();

	uint16_t GetIndex(uint64_t a_nameHash);
}    // namespace CR::Engine::Audio::FXLibrary

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

	struct FXDataSource {
		ma_data_source_base base;
		CR::Engine::Core::Buffer pcmData;
		uint32_t frameOffset{};
	};
	std::vector<FXDataSource> m_dataSources;

	FXDataSource& GetDataSourceIndex(ma_data_source* maDataSource) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");
		ptrdiff_t dist = std::distance(m_dataSources.data(), (FXDataSource*)maDataSource);
		CR_ASSERT(dist > 0 && dist < (ptrdiff_t)m_dataSources.size(), "fx invalid get format request");
		return m_dataSources[dist];
	}

	ma_result DataSourceRead(ma_data_source* maDataSource, void* pFramesOut, ma_uint64 frameCount,
	                         ma_uint64* pFramesRead) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");
		FXDataSource& dataSource = GetDataSourceIndex(maDataSource);
		frameCount =
		    std::min<ma_uint64>(frameCount, dataSource.pcmData.size<int16_t>() - dataSource.frameOffset);
		if(frameCount == 0) { return MA_AT_END; }

		if(pFramesOut) { memcpy(pFramesOut, dataSource.pcmData.data(), frameCount * sizeof(int16_t)); }
		if(pFramesRead) { *pFramesRead = frameCount; }

		dataSource.frameOffset += (uint32_t)frameCount;

		return MA_SUCCESS;
	}

	ma_result DataSourceSeek(ma_data_source* maDataSource, ma_uint64 frameIndex) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");

		FXDataSource& dataSource = GetDataSourceIndex(maDataSource);
		if(frameIndex >= dataSource.pcmData.size<int16_t>()) { return MA_ERROR; }
		dataSource.frameOffset = (uint32_t)frameIndex;
		return MA_SUCCESS;
	}

	ma_result DataSourceGetDataFormat(ma_data_source* maDataSource, ma_format* pFormat, ma_uint32* pChannels,
	                                  ma_uint32* pSampleRate, ma_channel* pChannelMap,
	                                  [[maybe_unused]] size_t channelMapCap) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");

		if(pFormat) { *pFormat = ma_format_s16; }
		if(pChannels) { *pChannels = 1; }
		if(pSampleRate) { *pSampleRate = CR::Engine::Audio::Constants::c_sampleRate; }
		if(pChannelMap) { *pChannelMap = MA_CHANNEL_MONO; }

		return MA_SUCCESS;
	}

	ma_result DataSourceGetCursor(ma_data_source* maDataSource, ma_uint64* pCursor) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");

		FXDataSource& dataSource = GetDataSourceIndex(maDataSource);
		if(pCursor) *pCursor = dataSource.frameOffset;

		return MA_SUCCESS;
	}

	ma_result DataSourceGetLength(ma_data_source* maDataSource, ma_uint64* pLength) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");

		FXDataSource& dataSource = GetDataSourceIndex(maDataSource);
		if(pLength) *pLength = dataSource.pcmData.size<int16_t>();

		return MA_SUCCESS;
	}

	void DataSourceInit(FXDataSource& a_dataSource) {
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

void cea::FXLibrary::Initialize() {
	flatbuffers::Parser parser = ceasset::GetData(cecore::C_Hash64("Audio/soundfx.json"), SCHEMAS_SOUNDFX);
	auto sounds                = Flatbuffers::GetSoundFXs(parser.builder_.GetBufferPointer());
	for(const auto& soundfx : *sounds->soundfx()) {
		m_lookup[cecore::Hash64(soundfx->name()->c_str())] = (uint16_t)m_dataSources.size();
		m_names.push_back(soundfx->name()->c_str());
		m_paths.push_back(soundfx->path()->c_str());

		auto soundHandle = ceasset::GetHandle(cecore::Hash64(m_paths.back()));
		ceasset::Open(soundHandle);
		auto soundData = ceasset::GetData(soundHandle);

		uint32_t uncompressedSize{};
		auto metaData = [](void* pUserData, drflac_metadata* pMetadata) {
			if(pMetadata->type == DRFLAC_METADATA_BLOCK_TYPE_STREAMINFO) {
				uint32_t* size = (uint32_t*)pUserData;
				*size          = (uint32_t)pMetadata->data.streaminfo.totalPCMFrameCount;
			}
		};
		auto drFlac = drflac_open_memory_with_metadata(soundData.data(), soundData.size(), metaData,
		                                               &uncompressedSize, nullptr);

		CR_ASSERT(uncompressedSize > 0, "0 size flac file");

		CR::Engine::Core::Buffer pcmData;
		pcmData.resize<int16_t>(uncompressedSize);

		auto framesRead = drflac_read_pcm_frames_s16(drFlac, uncompressedSize, pcmData.data<int16_t>());
		while(framesRead < uncompressedSize) {
			framesRead += drflac_read_pcm_frames_s16(drFlac, uncompressedSize - framesRead,
			                                         (int16_t*)pcmData.data() + framesRead);
		}
		drflac_close(drFlac);

		auto& dataSource = m_dataSources.emplace_back();
		DataSourceInit(dataSource);
		dataSource.pcmData = std::move(pcmData);

		ceasset::Close(soundHandle);
	}
}

void cea::FXLibrary::Shutdown() {
	for(auto& dataSource : m_dataSources) { ma_data_source_uninit(&dataSource.base); }
}

uint16_t cea::FXLibrary::GetIndex(uint64_t a_nameHash) {
	auto iter = m_lookup.find(a_nameHash);
	CR_ASSERT(iter != m_lookup.end(), "Could not find audio fx asset {}", a_nameHash);
	return iter->second;
}
