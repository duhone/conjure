module;

#include "core/Log.h"

#include <zstd.h>

module CR.Engine.Compression.General;

import CR.Engine.Core.Literals;

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

cec::StorageBuffer<std::byte> cecomp::General::Compress(const std::span<const std::byte> a_src,
                                                        int32_t a_level) {
	cec::StorageBuffer<std::byte> result;

	int bound = (int)ZSTD_compressBound(a_src.size());

	result.prepare(bound + sizeof(CompressionHeader));
	auto header              = new(std::data(result)) CompressionHeader;
	header->DecompressedSize = (uint32_t)a_src.size();

	header->CompressedSize = (uint32_t)ZSTD_compress(result.data() + sizeof(CompressionHeader), bound,
	                                                 (char*)a_src.data(), a_src.size(), a_level);

	result.commit(header->CompressedSize + sizeof(CompressionHeader));

	return result;
}

cec::StorageBuffer<std::byte> cecomp::General::Decompress(const std::span<const std::byte> a_src) {
	cec::StorageBuffer<std::byte> result;
	if(a_src.size() < sizeof(CompressionHeader)) return result;

	auto* header = reinterpret_cast<const CompressionHeader*>(a_src.data());
	CR_REQUIRES(header->FourCC == c_CompFourCC, "compressed data has invalid fourcc");
	CR_REQUIRES(header->Version == c_CompVersion, "compressed data has invalid version");
	CR_REQUIRES(header->CompressedSize <= a_src.size() - sizeof(CompressionHeader),
	            "compressed data size does not match header");

	result.prepare(header->DecompressedSize);
	int bytesRead =
	    (int)ZSTD_decompress(data(result), header->DecompressedSize,
	                         (char*)a_src.data() + sizeof(CompressionHeader), header->CompressedSize);

	CR_REQUIRES(bytesRead == (int)header->DecompressedSize, "did not decompress the expected amount of data");

	result.commit(header->DecompressedSize);

	return result;
}
