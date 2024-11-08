module;

#include "generated/graphics/sprites_generated.h"

#include "flatbuffers/idl.h"

#include <core/Log.h>

#include "ankerl/unordered_dense.h"

export module CR.Engine.Graphics.SpritesInternal;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Handles;

import CR.Engine.Assets;
import CR.Engine.Core;

import <vector>;

export namespace CR::Engine::Graphics::Sprites {
	// public API
	Handles::Sprite CreateInternal();
	void DeleteInternal(Handles::Sprite a_sprite);

	void Initialize();
	void Shutdown();

}    // namespace CR::Engine::Graphics::Sprites

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	struct Data {
		cecore::BitSet<cegraph::Constants::c_maxSprites> Used;

		// templates
		std::vector<std::string> TemplateNames;
		std::vector<uint64_t> TemplateHashes;
		std::vector<uint8_t> TemplateFPSDivisors;
		ankerl::unordered_dense::map<uint64_t, uint32_t> TemplateLookup;
	};

	Data* g_data = nullptr;
}    // namespace

cegraph::Handles::Sprite cegraph::Sprites::CreateInternal() {
	cegraph::Handles::Sprite result{g_data->Used.FindNotInSet()};
	CR_ASSERT(result.isValid(), "ran out of Sprites");
	g_data->Used.insert(result.asInt());

	return result;
}

void cegraph::Sprites::DeleteInternal(cegraph::Handles::Sprite a_sprite) {
	CR_ASSERT(g_data->Used.contains(a_sprite.asInt()), "tried to delete a sprite that doesn't exist");
	g_data->Used.erase(a_sprite.asInt());
}

void cegraph::Sprites::Initialize() {
	CR_ASSERT(g_data == nullptr, "Sprites are already initialized");

	g_data = new Data{};

	auto& assetService = cecore::GetService<ceasset::Service>();

	flatbuffers::Parser parser =
	    assetService.GetData(cecore::C_Hash64("Graphics/sprites.json"), SCHEMAS_SPRITES);
	auto spritesfb = Flatbuffers::GetSprites(parser.builder_.GetBufferPointer());

	const auto& sprites = *spritesfb->sprites();

	g_data->TemplateNames.reserve(sprites.size());
	g_data->TemplateHashes.reserve(sprites.size());
	g_data->TemplateFPSDivisors.reserve(sprites.size());
	for(uint32_t i = 0; i < sprites.size(); ++i) {
		g_data->TemplateNames.emplace_back(sprites[i]->name()->c_str());
		uint64_t hash = cecore::Hash64(sprites[i]->name()->c_str());
		g_data->TemplateHashes.emplace_back(hash);
		g_data->TemplateFPSDivisors.emplace_back(sprites[i]->frame_rate_divisor());

		g_data->TemplateLookup[hash] = i;
	}
}

void cegraph::Sprites::Shutdown() {
	CR_ASSERT(g_data != nullptr, "Sprites are already shutdown");
	CR_ASSERT(g_data->Used.empty(), "Sprites weren't all freed before shutdown");

	delete g_data;
}
