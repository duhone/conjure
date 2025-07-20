export module CR.Engine.Compression.Wav;

import CR.Engine.Core;
import CR.Engine.Platform;

import std;
import std.compat;

namespace CR::Engine::Compression::Wave {
	// Only 48Khz 16 bit mono uncompressed audio is supported.
	export std::unique_ptr<std::byte[]> Decompress(const std::span<const std::byte> a_inputData,
	                                               std::string_view a_debugName);
	export std::unique_ptr<std::byte[]> Decompress(const std::filesystem::path& a_inputPath);
}    // namespace CR::Engine::Compression::Wave

module :private;

#include "core/Core.h"

namespace cecore = CR::Engine::Core;
namespace ceplat = CR::Engine::Platform;
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

std::unique_ptr<std::byte[]> cecomp::Wave::Decompress(const std::span<const std::byte> a_inputData,
                                                      std::string_view a_debugName) {
	std::unique_ptr<std::byte[]> result;

	cecore::BinaryReader reader;
	reader.Data = a_inputData.data();
	reader.Size = (uint32_t)a_inputData.size();

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
			result = std::make_unique_for_overwrite<std::byte[]>(header.ChunkSize);
			memcpy(result.get(), reader.Data + reader.Offset, header.ChunkSize);
			reader.Offset += header.ChunkSize;
		} else {
			reader.Offset += header.ChunkSize;
		}
	}

	CR_ASSERT_ALWAYS(wavChunk.WavID == WavChunk::c_WavID,
	                 "Input wave file {} did not have expected riff chunk, invalid wave file", a_debugName);
	CR_ASSERT_ALWAYS(fmtChunk.FormatCode == 1, "Input wave file {} was not in raw pcm format", a_debugName);
	CR_ASSERT_ALWAYS(fmtChunk.NChannels == 1, "Input wave file {} not either mono or stereo", a_debugName);
	CR_ASSERT_ALWAYS(fmtChunk.BitsPerSample == 16, "Input wave file {} not 16 bits per sample", a_debugName);
	CR_ASSERT_ALWAYS(fmtChunk.SampleRate == 48000, "Input wave file {} not 48Khz", a_debugName);

	return result;
}

std::unique_ptr<std::byte[]> cecomp::Wave::Decompress(const fs::path& a_inputPath) {
	ceplat::MemoryMappedFile inputData(a_inputPath);
	return Decompress(inputData.GetData(), a_inputPath.string());
}