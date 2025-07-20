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

std::unique_ptr<std::byte[]> cecomp::General::Compress(const std::span<const std::byte> a_src,
                                                       int32_t a_level) {
	std::unique_ptr<std::byte[]> result;

	int bound = (int)ZSTD_compressBound(a_src.size());

	result                   = std::make_unique_for_overwrite<std::byte[]>(bound + sizeof(CompressionHeader));
	auto header              = new(result.get()) CompressionHeader;
	header->DecompressedSize = (uint32_t)a_src.size();

	header->CompressedSize = (uint32_t)ZSTD_compress(result.get() + sizeof(CompressionHeader), bound,
	                                                 (char*)a_src.data(), a_src.size(), a_level);

	uint32_t finalSize = header->CompressedSize + sizeof(CompressionHeader);
	auto minResult     = std::make_unique_for_overwrite<std::byte[]>(finalSize);
	std::memcpy(minResult.get(), result.get(), finalSize);

	return minResult;
}

std::unique_ptr<std::byte[]> cecomp::General::Decompress(const std::span<const std::byte> a_src) {
	std::unique_ptr<std::byte[]> result;
	if(a_src.size() < sizeof(CompressionHeader)) return result;

	auto* header = reinterpret_cast<const CompressionHeader*>(a_src.data());
	CR_ASSERT_ALWAYS(header->FourCC == c_CompFourCC, "compressed data has invalid fourcc");
	CR_ASSERT_ALWAYS(header->Version == c_CompVersion, "compressed data has invalid version");
	CR_ASSERT_ALWAYS(header->CompressedSize <= a_src.size() - sizeof(CompressionHeader),
	                 "compressed data size does not match header");

	result = std::make_unique_for_overwrite<std::byte[]>(header->DecompressedSize);
	int bytesRead =
	    (int)ZSTD_decompress(result.get(), header->DecompressedSize,
	                         (char*)a_src.data() + sizeof(CompressionHeader), header->CompressedSize);

	CR_ASSERT_ALWAYS(bytesRead == (int)header->DecompressedSize,
	                 "did not decompress the expected amount of data");

	uint32_t finalSize = header->DecompressedSize;
	auto minResult     = std::make_unique_for_overwrite<std::byte[]>(finalSize);
	std::memcpy(minResult.get(), result.get(), finalSize);

	return minResult;
}
