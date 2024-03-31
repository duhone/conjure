module;

#include "core/Log.h"

#include "ankerl/unordered_dense.h"

#include "flatbuffers/idl.h"

export module CR.Engine.Assets.Service;

import CR.Engine.Core;
import CR.Engine.Platform;

import <filesystem>;
import <functional>;
import <ranges>;
import <span>;
import <string_view>;
import <typeindex>;
import <vector>;

namespace CR::Engine::Assets {
	export class Service {
	  public:
		static inline constexpr uint64_t s_typeIndex = CR::Engine::Core::EightCC("EAstServ");

		Service(std::filesystem::path a_assetsFolder);
		Service(const Service&) = delete;
		Service(Service&&)      = delete;

		Service& operator=(const Service&) = delete;
		Service& operator=(Service&&)      = delete;

		using AssetHandle = CR::Engine::Core::Handle<class AssetHandleTag>;

		AssetHandle GetHandle(uint64_t a_hash);

		// Note, if the asset is bulk, then this will return an empty span unless you have the asset open
		std::span<std::byte> GetData(AssetHandle a_handle) { return m_spans[a_handle.asInt()]; }
		// convenience, so dont need to call both gethandle and getData. If your going to call GetData more
		// than once for the same hash, it will be faster to call the 2 separate functions rather than this
		// convenience function.
		std::span<std::byte> GetData(uint64_t a_hash) { return m_spans[GetHandle(a_hash).asInt()]; }

		// convenience when loading loose flatbuffers(json) where you need to provide the schema.
		flatbuffers::Parser GetData(AssetHandle a_handle, std::string_view a_schemaPath);
		flatbuffers::Parser GetData(uint64_t a_hash, std::string_view a_schemaPath) {
			return GetData(GetHandle(a_hash), a_schemaPath);
		}

		// a no-op if this isn't a bulk asset. You must call close as many times as you called open.
		void Open(AssetHandle a_handle);
		void Close(AssetHandle a_handle);

		const std::filesystem::path& GetRootPath() const { return m_assetsFolder; }

	  private:
		bool isBulkFile(std::string_view ext);

		std::filesystem::path m_assetsFolder;

		ankerl::unordered_dense::map<uint64_t, AssetHandle> m_handleLookup;

		std::vector<bool> m_isBulk;
		std::vector<std::filesystem::path> m_fullPaths;
		std::vector<std::string> m_debugFiles;
		// For non bulk assets, these spans are aways valid, for bulk, they will only be valid when the asset
		// is open.
		std::vector<std::span<std::byte>> m_spans;
		std::vector<uint16_t> m_openCount;

		// non bulk files will be all put into this buffer, when using source instead of packed.
		CR::Engine::Core::StorageBuffer<std::byte> m_looseData;

		std::vector<CR::Engine::Platform::MemoryMappedFile> m_bulkData;
	};
}    // namespace CR::Engine::Assets

module :private;

namespace cecore   = CR::Engine::Core;
namespace ceplat   = CR::Engine::Platform;
namespace ceassets = CR::Engine::Assets;

namespace fs = std::filesystem;

namespace {
	constexpr std::string_view c_bulkExtensions[] = {"flac", "jxl"};
}

ceassets::Service::Service(std::filesystem::path a_assetsFolder) : m_assetsFolder(a_assetsFolder) {
	std::vector<std::pair<size_t, size_t>> spanOffsets;
	for(const auto& dirEntry : fs::recursive_directory_iterator(a_assetsFolder)) {
		auto relativePath = fs::relative(dirEntry.path(), a_assetsFolder);
		relativePath      = relativePath.lexically_normal();

		if(dirEntry.is_regular_file()) {
			AssetHandle handle{m_isBulk.size()};

			m_fullPaths.emplace_back(dirEntry.path());
			auto pathToHash = relativePath.string();
			// we always use posix seperators
			std::ranges::replace(pathToHash, '\\', '/');
			m_debugFiles.emplace_back(pathToHash);
			m_openCount.emplace_back();

			auto ext = relativePath.extension().string();
			CR_ASSERT(ext.size() > 1, "missing extension: {}", ext);
			ext.erase(begin(ext));    // remove leading .
			bool isBulk = isBulkFile(ext);
			m_isBulk.emplace_back(isBulk);

			m_handleLookup.emplace(cecore::Hash64(pathToHash), handle);

			if(isBulk) {
				// not much to do, just mark it an unavailable.
				spanOffsets.emplace_back();
			} else {
				size_t fileSize = dirEntry.file_size();
				size_t newSize  = m_looseData.size() + fileSize;
				m_looseData.prepare(newSize);

				cecore::FileHandle fileHandle(dirEntry.path(), false);
				fread(m_looseData.data() + m_looseData.size(), 1, fileSize, fileHandle.asFile());
				spanOffsets.emplace_back(m_looseData.size(), fileSize);
				m_looseData.commit(newSize);
			}
		}
	}

	for(uint32_t i = 0; i < m_isBulk.size(); ++i) {
		if(m_isBulk[i]) {
			m_spans.emplace_back();
		} else {
			m_spans.emplace_back(m_looseData.data() + spanOffsets[i].first, spanOffsets[i].second);
		}
	}

	m_bulkData.resize(m_isBulk.size());
}

ceassets::Service::AssetHandle ceassets::Service::GetHandle(uint64_t a_hash) {
	auto result = m_handleLookup.find(a_hash);
	if(result == m_handleLookup.end()) { return AssetHandle{}; }
	return result->second;
}

bool ceassets::Service::isBulkFile(std::string_view ext) {
	for(const auto& bulkExt : c_bulkExtensions) {
		if(ext == bulkExt) { return true; }
	}
	return false;
}

void ceassets::Service::Open(AssetHandle a_handle) {
	CR_ASSERT(a_handle.isValid(), "can't open an invalid asset");
	CR_ASSERT(m_isBulk[a_handle.asInt()], "can only open bulk assets");

	if(m_openCount[a_handle.asInt()] == 0) {
		m_bulkData[a_handle.asInt()] = ceplat::MemoryMappedFile(m_fullPaths[a_handle.asInt()]);
		m_spans[a_handle.asInt()]    = m_bulkData[a_handle.asInt()].GetData();
	}
	++m_openCount[a_handle.asInt()];
}

void ceassets::Service::Close(AssetHandle a_handle) {
	CR_ASSERT(a_handle.isValid(), "can't close an invalid asset");
	CR_ASSERT(m_isBulk[a_handle.asInt()], "can only close bulk assets");
	CR_ASSERT(m_openCount[a_handle.asInt()] != 0, "can only close files that are open");

	if(m_openCount[a_handle.asInt()] == 1) {
		m_bulkData[a_handle.asInt()] = ceplat::MemoryMappedFile{};
		m_spans[a_handle.asInt()]    = std::span<std::byte>{};
	}
	--m_openCount[a_handle.asInt()];
}

flatbuffers::Parser ceassets::Service::GetData(AssetHandle a_handle, std::string_view a_schemaPath) {
	flatbuffers::Parser parser;
	ceplat::MemoryMappedFile schemaFile(a_schemaPath);
	std::string schemaData((const char*)schemaFile.data(), schemaFile.size());
	parser.Parse(schemaData.c_str());

	auto jsonData = GetData(a_handle);
	std::string flatbufferJson((const char*)jsonData.data(), jsonData.size());
	parser.ParseJson(flatbufferJson.c_str());
	CR_ASSERT(parser.BytesConsumed() <= (ptrdiff_t)jsonData.size(), "buffer overrun loading {}",
	          m_debugFiles[a_handle.asInt()]);
	return parser;
}
