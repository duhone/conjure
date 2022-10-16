module;

#include "core/Log.h"

export module CR.Engine.Assets.Service;

import CR.Engine.Core;

import CR.Engine.Assets.Partition;

import <filesystem>;
import <functional>;
import <span>;
import <string_view>;
import <typeindex>;

namespace CR::Engine::Assets {
	export class Service {
	  public:
		static std::type_index s_typeIndex;

		enum Partitions { Audio };

		Service(std::filesystem::path a_assetsFolder);
		Service(const Service&) = delete;
		Service(Service&&)      = delete;

		Service& operator=(const Service&) = delete;
		Service& operator=(Service&&)      = delete;

		using LoadCallbackT =
		    std::function<void(uint64_t a_hash, std::string_view a_path, const std::span<std::byte> a_data)>;
		void Load(Partitions a_partition, const std::filesystem::path& a_subFolder,
		          std::string_view a_extensionFilter, LoadCallbackT a_loadCallback);

	  private:
		std::vector<Partition> m_partitions;
	};
}    // namespace CR::Engine::Assets

module :private;

namespace cecore   = CR::Engine::Core;
namespace ceassets = CR::Engine::Assets;

namespace fs = std::filesystem;

std::type_index ceassets::Service::s_typeIndex{typeid(Service)};

namespace {
	constexpr std::string_view c_partitionFolders[] = {"Audio"};
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
