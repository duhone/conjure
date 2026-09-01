module;

#include "core/Log.h"

#include "ankerl/unordered_dense.h"

#include "flatbuffers/idl.h"

export module CR.Engine.Assets;

import CR.Engine.Core;
import CR.Engine.Platform;

import std;
import std.compat;

export namespace CR::Engine::Assets {
	namespace Handles {
		using Asset = CR::Engine::Core::Handle<class AssetHandleTag>;
	}    // namespace Handles

	void Initialize(const std::filesystem::path& a_gameAssetsFolder);
	void Shutdown();

	Handles::Asset GetHandle(uint64_t a_hash);

	// Note, if the asset is bulk, then this will return an empty span unless you have the asset open
	std::span<std::byte> GetData(Handles::Asset a_handle);
	// convenience, so dont need to call both gethandle and getData. If your going to call GetData more
	// than once for the same hash, it will be faster to call the 2 separate functions rather than this
	// convenience function.
	std::span<std::byte> GetData(uint64_t a_hash);

	// convenience when loading loose flatbuffers(json) where you need to provide the schema.
	flatbuffers::Parser GetData(Handles::Asset a_handle, std::string_view a_schemaPath);
	flatbuffers::Parser GetData(uint64_t a_hash, std::string_view a_schemaPath);

	// a no-op if this isn't a bulk asset. You must call close as many times as you called open.
	void Open(Handles::Asset a_handle);
	void Close(Handles::Asset a_handle);

	// These 2 only have meaning with loose assets. be careful in there use.
	// Intended for things like a shader that needs to be compiled in loose asset mode, but would be
	// pre-compiled in packed mode. So you would only need the full path in loose mode anyway.
	const std::filesystem::path& GetEngineRootPath();
	const std::filesystem::path& GetGameRootPath();

}    // namespace CR::Engine::Assets

module :private;

namespace cecore   = CR::Engine::Core;
namespace ceplat   = CR::Engine::Platform;
namespace ceassets = CR::Engine::Assets;

namespace fs = std::filesystem;

namespace {
	constexpr std::string_view c_bulkExtensions[] = {"flac", "jxl", "webp"};

	std::filesystem::path m_gameAssetsFolder;

	ankerl::unordered_dense::map<uint64_t, ceassets::Handles::Asset> m_handleLookup;

	std::vector<bool> m_isBulk;
	std::vector<std::filesystem::path> m_fullPaths;
	std::vector<std::string> m_debugFiles;
	// For non bulk assets, these spans are aways valid, for bulk, they will only be valid when the asset
	// is open.
	std::vector<std::span<std::byte>> m_spans;
	std::vector<uint16_t> m_openCount;

	// non bulk files will be all put into this buffer, when using source instead of packed.
	CR::Engine::Core::Buffer m_looseData;

	std::vector<CR::Engine::Platform::MemoryMappedFile> m_bulkData;
	// Could get more parallelism by having one mutex per bulk data. But I worry thats too many mutexes.
	// Hopefully it doesn't take long to open/close. I don't think we need to lock for other use cases.
	std::mutex m_bulkMutex;

	bool isBulkFile(std::string_view ext) {
		for(const auto& bulkExt : c_bulkExtensions) {
			if(ext == bulkExt) { return true; }
		}
		return false;
	}

	void AddAssets(const std::filesystem::path& a_gameAssetsFolder) {
		for(const auto& dirEntry : fs::recursive_directory_iterator(a_gameAssetsFolder)) {
			auto relativePath = fs::relative(dirEntry.path(), a_gameAssetsFolder);
			relativePath      = relativePath.lexically_normal();

			if(dirEntry.is_regular_file()) {
				ceassets::Handles::Asset handle{m_isBulk.size()};

				m_fullPaths.emplace_back(dirEntry.path());
				auto pathToHash = relativePath.string();
				// we always use posix separators
				std::ranges::replace(pathToHash, '\\', '/');
				m_debugFiles.emplace_back(pathToHash);
				m_openCount.emplace_back();

				auto ext = relativePath.extension().string();
				CR_ASSERT(ext.size() > 1, "missing extension: {}", ext);
				ext.erase(begin(ext));    // remove leading .
				bool isBulk = isBulkFile(ext);
				m_isBulk.emplace_back(isBulk);

				auto assetHash = cecore::Hash64(pathToHash);
				CR_ASSERT(m_handleLookup.find(assetHash) == m_handleLookup.end(), "duplicate asset: {}",
				          pathToHash);
				m_handleLookup.emplace(assetHash, handle);
			}
		}
	}
}    // namespace

void ceassets::Initialize(const std::filesystem::path& a_gameAssetsFolder) {
	m_gameAssetsFolder = a_gameAssetsFolder;

	AddAssets(GetEngineRootPath());
	AddAssets(GetGameRootPath());

	std::vector<std::pair<size_t, size_t>> spanOffsets;

	for(uint32_t i = 0; i < m_isBulk.size(); ++i) {
		if(m_isBulk[i]) {
			// not much to do, just mark it an unavailable.
			spanOffsets.emplace_back();
		} else {
			uint32_t fileSize = (uint32_t)fs::file_size(m_fullPaths[i]);
			uint32_t oldSize  = m_looseData.size();
			uint32_t newSize  = m_looseData.size() + fileSize;
			m_looseData.resize(newSize);

			cecore::FileHandle fileHandle(m_fullPaths[i], false);
			fread(m_looseData.data() + oldSize, 1, fileSize, fileHandle.asFile());
			spanOffsets.emplace_back(oldSize, fileSize);
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

void ceassets::Shutdown() {}

ceassets::Handles::Asset ceassets::GetHandle(uint64_t a_hash) {
	auto result = m_handleLookup.find(a_hash);
	CR_ASSERT(result != m_handleLookup.end(), "Could not find asset");
	if(result == m_handleLookup.end()) { return Handles::Asset{}; }
	return result->second;
}

std::span<std::byte> ceassets::GetData(Handles::Asset a_handle) {
	return m_spans[a_handle];
}

std::span<std::byte> ceassets::GetData(uint64_t a_hash) {
	return m_spans[GetHandle(a_hash)];
}

flatbuffers::Parser ceassets::GetData(Handles::Asset a_handle, std::string_view a_schemaPath) {
	flatbuffers::Parser parser;
	ceplat::MemoryMappedFile schemaFile(a_schemaPath);
	std::string schemaData((const char*)schemaFile.data(), schemaFile.size());
	parser.Parse(schemaData.c_str());

	auto jsonData = GetData(a_handle);
	std::string flatbufferJson((const char*)jsonData.data(), jsonData.size());
	parser.ParseJson(flatbufferJson.c_str());
	CR_ASSERT(parser.BytesConsumed() <= (ptrdiff_t)jsonData.size(), "buffer overrun loading {}",
	          m_debugFiles[a_handle]);
	return parser;
}

flatbuffers::Parser ceassets::GetData(uint64_t a_hash, std::string_view a_schemaPath) {
	return GetData(GetHandle(a_hash), a_schemaPath);
}

void ceassets::Open(Handles::Asset a_handle) {
	CR_ASSERT(a_handle.isValid(), "can't open an invalid asset");
	CR_ASSERT(m_isBulk[a_handle], "can only open bulk assets");

	// I don't think there is any reason to get fancy here. I don't expect multiple open requests for a
	// particular asset.
	std::scoped_lock lock(m_bulkMutex);

	if(m_openCount[a_handle] == 0) {
		m_bulkData[a_handle] = ceplat::MemoryMappedFile(m_fullPaths[a_handle]);
		m_spans[a_handle]    = m_bulkData[a_handle].GetData();
	}
	++m_openCount[a_handle];
}

void ceassets::Close(Handles::Asset a_handle) {
	CR_ASSERT(a_handle.isValid(), "can't close an invalid asset");
	CR_ASSERT(m_isBulk[a_handle], "can only close bulk assets");

	std::scoped_lock lock(m_bulkMutex);

	CR_ASSERT(m_openCount[a_handle] != 0, "can only close files that are open");

	if(m_openCount[a_handle] == 1) {
		m_bulkData[a_handle] = ceplat::MemoryMappedFile{};
		m_spans[a_handle]    = std::span<std::byte>{};
	}
	--m_openCount[a_handle];
}

const std::filesystem::path& ceassets::GetEngineRootPath() {
	static std::filesystem::path engineRootPath = ASSETS_FOLDER;
	return engineRootPath;
}

const std::filesystem::path& ceassets::GetGameRootPath() {
	return m_gameAssetsFolder;
}