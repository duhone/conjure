module;

#include "core/Log.h"

export module CR.Engine.Assets.Partition;

import <filesystem>;

namespace CR::Engine::Assets {
	export class Partition {
	  public:
		Partition(std::filesystem::path a_folder);
		Partition(const Partition&) = delete;
		Partition(Partition&&)      = default;

		Partition& operator=(const Partition&) = delete;
		Partition& operator=(Partition&&)      = default;
	};
}    // namespace CR::Engine::Assets

module :private;

namespace ceassets = CR::Engine::Assets;

namespace fs = std::filesystem;

ceassets::Partition::Partition(std::filesystem::path a_folder) {}
