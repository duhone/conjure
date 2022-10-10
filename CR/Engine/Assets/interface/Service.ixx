module;

#include "core/Log.h"

export module CR.Engine.Assets.Service;

import CR.Engine.Core;

import CR.Engine.Assets.Partition;

import <filesystem>;
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