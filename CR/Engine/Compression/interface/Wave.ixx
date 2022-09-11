export module CR.Engine.Compression.Wav;

import CR.Engine.Core;
import CR.Engine.Platform;

import<cstddef>;
import<filesystem>;
import<memory>;
import<span>;

namespace CR::Engine::Compression::Wave {
	// Only 48Khz 16 bit mono uncompressed audio is supported.
	export CR::Engine::Core::StorageBuffer<int16_t> Decompress(const std::filesystem::path& a_inputPath);
}    // namespace CR::Engine::Compression::Wave

module : private;

#include "core/Log.h"

namespace cec    = CR::Engine::Core;
namespace cep    = CR::Engine::Platform;
namespace cecomp = CR::Engine::Compression;

namespace fs = std::filesystem;

struct ChunkHeader {
	uint32_t ChunkID;
	uint32_t ChunkSize;
};

struct WavChunk {
	const static uint32_t c_ChunkID = 'FFIR';

	const static uint32_t c_WavID = 'EVAW';

	uint32_t WavID{0};
};

struct FmtChunk {
	const static uint32_t c_ChunkID = ' tmf';

	uint16_t FormatCode{0};
	uint16_t NChannels{0};
	uint32_t SampleRate{0};
	uint32_t DataRate{0};
	uint16_t DataBlockSize{0};
	uint16_t BitsPerSample{0};
};

static_assert(sizeof(FmtChunk) == 16);

CR::Engine::Core::StorageBuffer<int16_t> cecomp::Wave::Decompress(const fs::path& a_inputPath) {
	CR::Engine::Core::StorageBuffer<int16_t> result;

	cep::MemoryMappedFile inputData(a_inputPath);

	cec::BinaryReader reader;
	reader.Data = inputData.data();
	reader.Size = (uint32_t)inputData.size();

	WavChunk wavChunk;
	FmtChunk fmtChunk;

	ChunkHeader header;
	while(Read(reader, header)) {
		if(header.ChunkID == WavChunk::c_ChunkID) {
			Read(reader, wavChunk);    // ignore chunk size for main header
		} else if(header.ChunkID == FmtChunk::c_ChunkID) {
			Read(reader, fmtChunk);
			reader.Offset += header.ChunkSize - sizeof(FmtChunk);    // skip extended fmts
		} else if(header.ChunkID == 'atad') {
			// handle data chunk specially
			result.prepare(header.ChunkSize / 2);
			memcpy(result.data(), reader.Data + reader.Offset, header.ChunkSize / 2);
			result.commit(header.ChunkSize / 2);
			reader.Offset += header.ChunkSize;
		} else {
			reader.Offset += header.ChunkSize;
		}
	}

	CR_ASSERT(wavChunk.WavID == WavChunk::c_WavID,
	          "Input wave file {} did not have expected riff chunk, invalid wave file", a_inputPath.string());
	CR_ASSERT(fmtChunk.FormatCode == 1, "Input wave file {} was not in raw pcm format", a_inputPath.string());
	CR_ASSERT(fmtChunk.NChannels == 1, "Input wave file {} not either mono or stereo", a_inputPath.string());
	CR_ASSERT(fmtChunk.BitsPerSample == 16, "Input wave file {} not 16 bits per sample",
	          a_inputPath.string());
	CR_ASSERT(fmtChunk.SampleRate == 48000, "Input wave file {} not 48Khz", a_inputPath.string());

	return result;
}