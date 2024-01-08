module;

#include "core/Log.h"

#include "ankerl/unordered_dense.h"

export module CR.Engine.Assets.Service;

import CR.Engine.Core;

import <filesystem>;
import <functional>;
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

		// Not if the asset is bulk, then this will return an empty span unless you have the asset open
		std::span<std::byte> GetData(uint64_t a_hash);

		// a no-op if this isn't a bulk asset. You must call close as many times as you called open.
		void Open(uint64_t a_hash);
		void Close(uint64_t a_hash);

	  private:
		ankerl::unordered_dense::map<uint64_t, uint16_t> m_handleLookup;

		std::vector<bool> m_isBulk;
		std::vector<std::string> m_debugFiles;
		// For non bulk assets, these spans are aways valid, for bulk, they will only be valid when the asset
		// is open.
		std::vector<std::span<std::byte>> m_spans;

		// non bulk files will be all put into this buffer, when using source instead of packed.
		std::vector<std::byte> m_looseData;
	};
}    // namespace CR::Engine::Assets

module :private;

namespace cecore   = CR::Engine::Core;
namespace ceassets = CR::Engine::Assets;

namespace fs = std::filesystem;

namespace {
	constexpr std::string_view c_bulkExtensions[] = {"flac", "jxl"};
}

ceassets::Service::Service(std::filesystem::path a_assetsFolder) {
	for(const auto& folder : c_partitionFolders) {
		fs::path partitionFolder = a_assetsFolder / folder;
		m_partitions.emplace_back(partitionFolder);
	}
}

void ceassets::Service::Load(Partitions a_partition, const std::filesystem::path& a_subFolder,
                             std::string_view a_extensionFilter, LoadCallbackT a_loadCallback) {
	CR_ASSERT(a_partition < m_partitions.size(), "Invalid partition");
	m_partitions[a_partition].Load(a_subFolder, a_extensionFilter, std::move(a_loadCallback));
}

void ceassets::Service::LoadSingle(Partitions a_partition, uint64_t a_hash, LoadCallbackT a_loadCallback) {
	CR_ASSERT(a_partition < m_partitions.size(), "Invalid partition");
	m_partitions[a_partition].LoadSingle(a_hash, std::move(a_loadCallback));
}