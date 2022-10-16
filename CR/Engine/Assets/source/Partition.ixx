module;

#include "core/Log.h"

export module CR.Engine.Assets.Partition;

import CR.Engine.Core;

import <algorithm>;
import <filesystem>;
import <ranges>;
import <vector>;

namespace CR::Engine::Assets {
	export class Partition {
	  public:
		Partition(std::filesystem::path a_rootFolder);
		Partition(const Partition&) = delete;
		Partition(Partition&&)      = default;

		Partition& operator=(const Partition&) = delete;
		Partition& operator=(Partition&&)      = default;

	  private:
		struct Folder {
			std::vector<uint64_t> Hashes;
			std::vector<std::string> Files;
			std::vector<std::string> Extensions;

			std::vector<std::string> FolderNames;
			std::vector<Folder> Folders;
		};

		void BuildFolder(const std::filesystem::path& a_rootPath, const std::filesystem::path& a_currentPath,
		                 Folder& a_currentFolder);

		Folder m_rootFolder;
	};
}    // namespace CR::Engine::Assets

module :private;

namespace cecore   = CR::Engine::Core;
namespace ceassets = CR::Engine::Assets;

namespace fs = std::filesystem;

ceassets::Partition::Partition(std::filesystem::path a_rootFolder) {
	BuildFolder(a_rootFolder, a_rootFolder, m_rootFolder);
}

void ceassets::Partition::BuildFolder(const fs::path& a_rootPath, const fs::path& a_currentPath,
                                      Folder& a_currentFolder) {
	for(const auto& dirEntry : fs::directory_iterator(a_currentPath)) {
		auto relativePath = fs::relative(dirEntry.path(), a_rootPath);
		relativePath      = relativePath.lexically_normal();

		if(dirEntry.is_regular_file()) {
			auto pathToHash = relativePath.string();
			// we always use posix seperators
			std::ranges::replace(pathToHash, '\\', '/');
			a_currentFolder.Hashes.emplace_back(cecore::Hash64(pathToHash));
			a_currentFolder.Files.emplace_back(std::move(pathToHash));
			auto ext = relativePath.extension().string();
			CR_ASSERT(ext.size() > 1, "missing extension: {}", ext);
			ext.erase(begin(ext));    // remove leading .
			a_currentFolder.Extensions.emplace_back(ext);
		} else if(dirEntry.is_directory()) {
			a_currentFolder.FolderNames.emplace_back(relativePath.stem().string());
			a_currentFolder.Folders.emplace_back();
			BuildFolder(a_rootPath, dirEntry.path(), a_currentFolder.Folders.back());
		}
	}
}