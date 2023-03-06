import CR.Engine.Core;
import CR.Engine.Platform;

import <chrono>;
import <cstdio>;

#include "core/Log.h"

#include <cli11/cli11.hpp>
#include <fmt/format.h>
#include <opus.h>

using namespace std::chrono_literals;
namespace fs  = std::filesystem;
namespace cec = CR::Engine::Core;
namespace cep = CR::Engine::Platform;

static const uint32_t c_SampleRate     = 48000;
static const uint32_t c_OpusBitRate    = 128000;
static const uint32_t c_OpusFrameSize  = 480;
static const uint32_t c_bytesPerSample = 2;
// for one channel
static const uint32_t c_OpusOutSamplesPerSec =
    (uint32_t)((((int64_t)c_OpusBitRate / 8) * c_OpusFrameSize) / c_SampleRate);
// extra 2x, allow opus to encode a single frame up to 2x the ideal size
static const uint32_t c_OpusOutFrameSizePerChannel = c_OpusOutSamplesPerSec * c_bytesPerSample * 2;

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

struct WavFile {
	uint32_t NChannels{0};
	std::vector<int16_t> Data;
};

#pragma pack(1)
struct CRAUDHeader {
	uint32_t FourCC{'CRAU'};
	uint16_t Version{1};
	uint16_t EncoderDelaySamples{0};
	uint32_t NSamples{0};
	uint8_t NChannels{0};
};
#pragma pack()

WavFile ReadWaveFile(const fs::path& a_inputPath) {
	WavFile result;

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
			result.Data.resize(header.ChunkSize / 2);
			memcpy(result.Data.data(), reader.Data + reader.Offset, header.ChunkSize / 2);
			reader.Offset += header.ChunkSize;
		} else {
			reader.Offset += header.ChunkSize;
		}
	}

	if(wavChunk.WavID != WavChunk::c_WavID) {
		CR_WARN("Input wave file {} did not have expected riff chunk, invalid wave file",
		        a_inputPath.string());
		result.Data.clear();
		return result;
	}

	if(fmtChunk.FormatCode != 1) {
		CR_WARN("Input wave file {} was not in raw pcm format", a_inputPath.string());
		result.Data.clear();
		return result;
	}

	if(!(fmtChunk.NChannels == 1 || fmtChunk.NChannels == 2)) {
		CR_WARN("Input wave file {} not either mono or stereo", a_inputPath.string());
		result.Data.clear();
		return result;
	}

	if(fmtChunk.BitsPerSample != 16) {
		CR_WARN("Input wave file {} not 16 bits per sample", a_inputPath.string());
		result.Data.clear();
		return result;
	}

	if(fmtChunk.SampleRate != 48000) {
		CR_WARN("Input wave file {} not 48Khz", a_inputPath.string());
		result.Data.clear();
		return result;
	}

	result.NChannels = fmtChunk.NChannels;

	return result;
}

int main(int argc, char** argv) {
	CLI::App app{"AudioProcessor"};
	std::string inputFileName  = "";
	std::string outputFileName = "";
	app.add_option("-i,--input", inputFileName,
	               "Input wav file, must be wav 48khz 16bit, stereo or mono supported.")
	    ->required();
	app.add_option("-o,--output", outputFileName, "Output craud file and path.")->required();

	CLI11_PARSE(app, argc, argv);

	fs::path inputPath{inputFileName};
	fs::path outputPath{outputFileName};

	fs::current_path(cep::GetCurrentProcessPath());

	if(!fs::exists(inputPath)) {
		CLI::Error error{"input file", "Input file doesn't exist", CLI::ExitCodes::FileError};
		app.exit(error);
	}
	if(outputPath.has_extension() && outputPath.extension() != ".craud") {
		CLI::Error error{"extension", "extension must be craud, or optionaly don't specify an extension",
		                 CLI::ExitCodes::FileError};
		app.exit(error);
	}

	outputPath.replace_extension(".craud");

	bool needsUpdating = false;
	if(!fs::exists(outputPath) || (fs::last_write_time(outputPath) <= fs::last_write_time(inputPath))) {
		needsUpdating = true;
	}
	if(!needsUpdating) {
		return 0;    // nothing to do;
	}

	{
		fs::path outputFolder = outputPath;
		outputFolder.remove_filename();
		if(!outputFolder.empty()) { fs::create_directories(outputFolder); }
	}

	WavFile pcmData = ReadWaveFile(inputPath);

	if(pcmData.Data.empty()) {
		CLI::Error error{"input file", "Failed to read input file, or input file was a wrong format",
		                 CLI::ExitCodes::FileError};
		app.exit(error);
	}

	int opusError        = 0;
	OpusEncoder* encoder = opus_encoder_create(48000, pcmData.NChannels, OPUS_APPLICATION_AUDIO, &opusError);

	if(encoder == nullptr || opusError != 0) {
		CLI::Error error{"input file", "Failed to create opus encoder", CLI::ExitCodes::HorribleError};
		app.exit(error);
	}

	uint32_t nSamples = (uint32_t)pcmData.Data.size() / pcmData.NChannels;

	opus_encoder_ctl(encoder, OPUS_SET_BITRATE(128000 * pcmData.NChannels));
	opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(9));
	int32_t encoderDelay = 0;
	opus_encoder_ctl(encoder, OPUS_GET_LOOKAHEAD(&encoderDelay));

	std::vector<std::byte> outBuffer;
	outBuffer.resize(c_OpusOutFrameSizePerChannel * pcmData.NChannels);

	uint32_t totalFrameSize = c_OpusFrameSize * pcmData.NChannels;

	uint32_t paddingBytes = (pcmData.Data.size() + encoderDelay) % totalFrameSize;
	paddingBytes          = paddingBytes == 0 ? 0 : totalFrameSize - paddingBytes;
	pcmData.Data.resize(pcmData.Data.size() + encoderDelay + paddingBytes);

	cec::FileHandle outFile(outputPath);
	{
		CRAUDHeader header;
		header.EncoderDelaySamples = (uint16_t)encoderDelay;
		header.NSamples            = nSamples;
		header.NChannels           = (uint8_t)pcmData.NChannels;

		Write(outFile, header);
	}

	uint32_t nframes = ((uint32_t)pcmData.Data.size() / totalFrameSize);
	for(uint32_t frame = 0; frame < nframes; ++frame) {
		int32_t bytesWritten =
		    opus_encode(encoder, pcmData.Data.data() + frame * totalFrameSize, c_OpusFrameSize,
		                (uint8_t*)outBuffer.data(), (int32_t)outBuffer.size());
		fwrite(outBuffer.data(), sizeof(std::byte), bytesWritten, outFile.asFile());
	}

	opus_encoder_destroy(encoder);

	return 0;
}
