module;

#include "core/Core.h"

#include <zstd.h>

module CR.Engine.Compression.General;

import CR.Engine.Core;

import std;
import std.compat;

namespace cec    = CR::Engine::Core;
namespace cecomp = CR::Engine::Compression;

using namespace CR::Engine::Core::Literals;

namespace {
	const int c_CompFourCC{'CRCF'};
	const int c_CompVersion{2};

#pragma pack(4)
	struct CompressionHeader {
		int FourCC{c_CompFourCC};
		uint32_t Version{c_CompVersion};
		uint32_t DecompressedSize{0};
		uint32_t CompressedSize{0};
	};
#pragma pack()
}    // namespace

CR::Engine::Core::Buffer cecomp::General::Compress(const std::span<const std::byte> a_src, int32_t a_level) {
	CR::Engine::Core::Buffer result;

	int bound = (int)ZSTD_compressBound(a_src.size());

	result.resize(bound + sizeof(CompressionHeader));
	auto header              = new(result.data()) CompressionHeader;
	header->DecompressedSize = (uint32_t)a_src.size();

	header->CompressedSize = (uint32_t)ZSTD_compress(result.data() + sizeof(CompressionHeader), bound,
	                                                 (char*)a_src.data(), a_src.size(), a_level);

	result.resize(header->CompressedSize + sizeof(CompressionHeader));

	return result;
}

CR::Engine::Core::Buffer cecomp::General::Decompress(const std::span<const std::byte> a_src) {
	CR::Engine::Core::Buffer result;
	if(a_src.size() < sizeof(CompressionHeader)) return result;

	auto* header = reinterpret_cast<const CompressionHeader*>(a_src.data());
	CR_ASSERT_ALWAYS(header->FourCC == c_CompFourCC, "compressed data has invalid fourcc");
	CR_ASSERT_ALWAYS(header->Version == c_CompVersion, "compressed data has invalid version");
	CR_ASSERT_ALWAYS(header->CompressedSize <= a_src.size() - sizeof(CompressionHeader),
	                 "compressed data size does not match header");

	result.resize(header->DecompressedSize);
	int bytesRead =
	    (int)ZSTD_decompress(result.data(), header->DecompressedSize,
	                         (char*)a_src.data() + sizeof(CompressionHeader), header->CompressedSize);

	CR_ASSERT_ALWAYS(bytesRead == (int)header->DecompressedSize,
	                 "did not decompress the expected amount of data");

	result.resize(header->DecompressedSize);

	return result;
}
