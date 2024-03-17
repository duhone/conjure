module;

#include "generated/graphics/textures_generated.h"

#include "flatbuffers/idl.h"

#include "core/Log.h"

#include "ankerl/unordered_dense.h"

#include "Core.h"

export module CR.Engine.Graphics.Textures;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.GraphicsThread;
import CR.Engine.Graphics.Utils;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import <cstring>;
import <filesystem>;

export namespace CR::Engine::Graphics::Textures {
	using TextureHandle = CR::Engine::Core::Handle<uint16_t, class TextureHandleTag>;

	void Initialize();
	void Shutdown();

	TextureHandle GetHandle(uint64_t hash);

}    // namespace CR::Engine::Graphics::Textures

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;
namespace ceplat  = CR::Engine::Platform;

namespace fs = std::filesystem;

namespace {
	struct Data {
		// Variables for in use textures

		// variables for all textures
		cecore::BitSet<cegraph::Constants::c_maxTextures> Used;
		std::array<std::string, cegraph::Constants::c_maxTextures> DebugNames;
		std::array<fs::path, cegraph::Constants::c_maxTextures> Paths;
		std::array<uint16_t, cegraph::Constants::c_maxTextures> RefCounts;

		// lookups
		ankerl::unordered_dense::map<uint64_t, cegraph::Textures::TextureHandle> m_handleLookup;
	};

	Data* g_data = nullptr;
}    // namespace

void cegraph::Textures::Initialize() {
	CR_ASSERT(g_data == nullptr, "Textures are already initialized");
	g_data = new Data{};

	auto& assetService   = cecore::GetService<ceasset::Service>();
	const auto& rootPath = assetService.GetRootPath();

	flatbuffers::Parser parser =
	    assetService.GetData(cecore::C_Hash64("Graphics/textures.json"), SCHEMAS_TEXTURES);

	auto texturesfb = Flatbuffers::GetTextures(parser.builder_.GetBufferPointer());

	const auto& textures = *texturesfb->textures();
	g_data->Used.insertRange(0, (uint16_t)textures.size());
	for(uint32_t i = 0; i < textures.size(); ++i) {
		TextureHandle handle{i};
		g_data->DebugNames[i] = textures[i]->name()->c_str();
		g_data->Paths[i]      = (rootPath / textures[i]->path()->c_str());

		g_data->m_handleLookup[cecore::Hash64(textures[i]->name()->c_str())] = handle;
	}

	memset(g_data->RefCounts.data(), 0, g_data->RefCounts.size() * sizeof(uint16_t));
}

void cegraph::Textures::Shutdown() {
	CR_ASSERT(g_data != nullptr, "Textures are already shutdown");
	delete g_data;
}

cegraph::Textures::TextureHandle cegraph::Textures::GetHandle(uint64_t hash) {
	CR_ASSERT(g_data != nullptr, "Textures not initialized");
	auto handleIter = g_data->m_handleLookup.find(hash);
	CR_ASSERT(handleIter != g_data->m_handleLookup.end(), "Texture could not be found");
	return handleIter->second;
}