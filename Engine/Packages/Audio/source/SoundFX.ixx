module;

#include "generated/audio/soundfx_generated.h"

#include "flatbuffers/idl.h"

#include "core/Log.h"

#include <dr_flac.h>
#include <miniaudio.h>

export module CR.Engine.Audio.SoundFX;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import CR.Engine.Audio.Constants;
import CR.Engine.Audio.Handles;

import std;
import std.compat;

export namespace CR::Engine::Audio::SoundFX {
	void Initialize(ma_engine& m_minAudio, ma_sound_group& a_soundGroup);
	void Shutdown();
	void Update();

	extern "C++" [[nodiscard]] Handles::SoundFX GetHandle(uint64_t a_nameHash);
	extern "C++" void Play(Handles::SoundFX a_handle);

}    // namespace CR::Engine::Audio::SoundFX

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace ceplat  = CR::Engine::Platform;
namespace ceaud   = CR::Engine::Audio;

namespace fs = std::filesystem;

namespace {
	ma_engine* m_minAudio;
	ma_sound_group* m_soundGroup{};
	ma_data_source_config m_baseConfig{};

	cecore::HandlePool<ceaud::Handles::SoundFX, ceaud::Constants::c_maxSoundFX> m_handlePool;
	std::unordered_map<uint64_t, ceaud::Handles::SoundFX> m_lookup;
	std::array<std::string, ceaud::Constants::c_maxSoundFX> m_names;
	std::array<std::string, ceaud::Constants::c_maxSoundFX> m_paths;

	// will only need in loose asset mode
	CR::Engine::Core::Buffer m_audioBuffer;
	// spans to audio data, either m_audioBuffer, or the packed audio buffer
	std::array<std::span<int16_t>, ceaud::Constants::c_maxSoundFX> m_buffers;

	struct FXDataSource {
		ma_data_source_base base;
		std::span<int16_t> pcmData;
		uint32_t frameOffset{};
	};
	cecore::BitSet<ceaud::Constants::c_maxSoundFXPlaying> m_activeDataSources;
	std::array<FXDataSource, ceaud::Constants::c_maxSoundFXPlaying> m_dataSources;
	std::array<ma_sound, ceaud::Constants::c_maxSoundFXPlaying> m_sounds;

	FXDataSource& GetDataSourceIndex(ma_data_source* maDataSource) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");
		ptrdiff_t dist = std::distance(m_dataSources.data(), (FXDataSource*)maDataSource);
		CR_ASSERT(dist >= 0 && dist < (ptrdiff_t)m_dataSources.size(), "fx invalid get format request");
		return m_dataSources[dist];
	}

	ma_result DataSourceRead(ma_data_source* maDataSource, void* pFramesOut, ma_uint64 frameCount,
	                         ma_uint64* pFramesRead) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");
		FXDataSource& dataSource = GetDataSourceIndex(maDataSource);
		frameCount = std::min<ma_uint64>(frameCount, dataSource.pcmData.size() - dataSource.frameOffset);
		if(frameCount == 0) { return MA_AT_END; }

		if(pFramesOut) {
			memcpy(pFramesOut, dataSource.pcmData.data() + dataSource.frameOffset,
			       frameCount * sizeof(int16_t));
		}
		if(pFramesRead) { *pFramesRead = frameCount; }

		dataSource.frameOffset += (uint32_t)frameCount;

		return MA_SUCCESS;
	}

	ma_result DataSourceSeek(ma_data_source* maDataSource, ma_uint64 frameIndex) {
		CR_ASSERT(maDataSource != nullptr, "miniaudio null data source");

		FXDataSource& dataSource = GetDataSourceIndex(maDataSource);
		if(frameIndex >= dataSource.pcmData.size()) { return MA_ERROR; }
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
		if(pLength) *pLength = dataSource.pcmData.size();

		return MA_SUCCESS;
	}

}    // namespace

void ceaud::SoundFX::Initialize(ma_engine& minAudio, ma_sound_group& a_soundGroup) {
	m_soundGroup = &a_soundGroup;
	m_minAudio   = &minAudio;

	flatbuffers::Parser parser = ceasset::GetData(cecore::C_Hash64("Audio/soundfx.json"), SCHEMAS_SOUNDFX);
	auto sounds                = Flatbuffers::GetSoundFXs(parser.builder_.GetBufferPointer());

	uint32_t totalUncompressedSize{};
	for(const auto& soundfx : *sounds->soundfx()) {
		auto soundHandle = ceasset::GetHandle(cecore::Hash64(soundfx->path()->c_str()));
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

		drflac_close(drFlac);

		CR_ASSERT(uncompressedSize > 0, "0 size flac file");
		uncompressedSize *= sizeof(int16_t);
		uncompressedSize = (uncompressedSize + 63) & 0xffffffc0;

		totalUncompressedSize += uncompressedSize;
	}

	m_audioBuffer.resize(totalUncompressedSize);
	int16_t* bufferPtr = m_audioBuffer.data<int16_t>();

	for(const auto& soundfx : *sounds->soundfx()) {
		auto handle                                        = m_handlePool.acquire();
		m_lookup[cecore::Hash64(soundfx->name()->c_str())] = handle;
		m_names[handle]                                    = soundfx->name()->c_str();
		m_paths[handle]                                    = soundfx->path()->c_str();

		auto soundHandle = ceasset::GetHandle(cecore::Hash64(m_paths[handle]));
		auto soundData   = ceasset::GetData(soundHandle);

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

		auto framesRead = drflac_read_pcm_frames_s16(drFlac, uncompressedSize, bufferPtr);
		while(framesRead < uncompressedSize) {
			framesRead +=
			    drflac_read_pcm_frames_s16(drFlac, uncompressedSize - framesRead, bufferPtr + framesRead);
		}
		drflac_close(drFlac);

		m_buffers[handle] = std::span<int16_t>{bufferPtr, uncompressedSize};
		uncompressedSize  = (uncompressedSize + 31) & 0xffffffe0;
		bufferPtr += uncompressedSize;

		ceasset::Close(soundHandle);
	}

	static ma_data_source_vtable dataSourceVtable = {DataSourceRead, DataSourceSeek, DataSourceGetDataFormat,
	                                                 DataSourceGetCursor, DataSourceGetLength};

	m_baseConfig        = ma_data_source_config_init();
	m_baseConfig.vtable = &dataSourceVtable;
}

void ceaud::SoundFX::Shutdown() {
	for(uint16_t slot : m_activeDataSources) {
		ma_sound_uninit(&m_sounds[slot]);
		ma_data_source_uninit(&m_dataSources[slot].base);
	}
	m_activeDataSources.clear();
}

void ceaud::SoundFX::Update() {
	cecore::BitSet<ceaud::Constants::c_maxSoundFXPlaying> finished;

	for(uint16_t slot : m_activeDataSources) {
		if(ma_sound_at_end(&m_sounds[slot])) { finished.insert(slot); }
	}

	for(uint16_t slot : finished) {
		ma_sound_uninit(&m_sounds[slot]);
		ma_data_source_uninit(&m_dataSources[slot].base);
	}
	m_activeDataSources = m_activeDataSources & ~finished;
}

extern "C++" ceaud::Handles::SoundFX ceaud::SoundFX::GetHandle(uint64_t a_nameHash) {
	auto iter = m_lookup.find(a_nameHash);
	CR_ASSERT(iter != m_lookup.end(), "Could not find audio fx asset {}", a_nameHash);
	return iter->second;
}

extern "C++" void ceaud::SoundFX::Play(Handles::SoundFX a_handle) {
	CR_ASSERT(m_handlePool.isValid(a_handle), "tried to play in an invalid soundfx");
	if(m_activeDataSources.full()) {
		CR_WARN("already playing max soundfx");
		return;
	}

	uint16_t slot = m_activeDataSources.FindNotInSet();
	m_activeDataSources.insert(slot);

	auto& dataSource       = m_dataSources[slot];
	dataSource.frameOffset = 0;
	dataSource.pcmData     = m_buffers[a_handle];

	ma_result result = ma_data_source_init(&m_baseConfig, &dataSource.base);
	CR_ASSERT(result == MA_SUCCESS, "failed to initialize data source");

	ma_sound& sound = m_sounds[slot];
	ma_sound_config soundConfig;

	soundConfig                                = ma_sound_config_init();
	soundConfig.pFilePath                      = nullptr;
	soundConfig.pDataSource                    = &dataSource;
	soundConfig.pInitialAttachment             = m_soundGroup;
	soundConfig.initialAttachmentInputBusIndex = 0;
	soundConfig.channelsIn                     = 1;
	soundConfig.channelsOut                    = 1;

	result = ma_sound_init_ex(m_minAudio, &soundConfig, &sound);
	CR_ASSERT(result == MA_SUCCESS, "failed to play sounds");

	result = ma_sound_start(&sound);
	CR_ASSERT(result == MA_SUCCESS, "failed to play sounds");
}
