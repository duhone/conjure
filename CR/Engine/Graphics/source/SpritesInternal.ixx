module;

#include "generated/graphics/sprites_generated.h"

#include "flatbuffers/idl.h"

#include <core/Log.h>

#include "ankerl/unordered_dense.h"
#include <glm/glm.hpp>

export module CR.Engine.Graphics.SpritesInternal;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Handles;
import CR.Engine.Graphics.Textures;

import CR.Engine.Assets;
import CR.Engine.Core;

import <array>;
import <span>;
import <vector>;

export namespace CR::Engine::Graphics::Sprites {
	// public API
	void CreateInternal(std::span<uint64_t> a_hashes, std::span<Handles::Sprite> handles);
	void DeleteInternal(std::span<Handles::Sprite> a_sprites);
	void SetPositionsInternal(std::span<Handles::Sprite> a_sprites, std::span<glm::vec2> a_positions);
	void SetRotationsInternal(std::span<Handles::Sprite> a_sprites, std::span<float> a_rotations);

	void Initialize();
	void Shutdown();
	void Update();

}    // namespace CR::Engine::Graphics::Sprites

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
	struct Data {
		cecore::BitSet<cegraph::Constants::c_maxSprites> Used;
		std::array<cegraph::Handles::Texture, cegraph::Constants::c_maxSprites> TextureHandles;
		std::array<glm::vec2, cegraph::Constants::c_maxSprites> Positions;
		std::array<float, cegraph::Constants::c_maxSprites> Rotations;
		// these aren't the display frame. its the display frame * fps divisor.
		std::array<uint32_t, cegraph::Constants::c_maxSprites> NumFrames;
		std::array<uint32_t, cegraph::Constants::c_maxSprites> CurrentFrame;
		std::array<uint16_t, cegraph::Constants::c_maxSprites> TemplateIndices;

		// templates
		std::vector<std::string> TemplateNames;
		std::vector<uint64_t> TemplateHashes;
		std::vector<uint8_t> TemplateFrameRates;
		std::vector<uint64_t> TemplateTextureHashes;
		ankerl::unordered_dense::map<uint64_t, uint16_t> TemplateLookup;
	};

	Data* g_data = nullptr;
}    // namespace

void cegraph::Sprites::CreateInternal(std::span<uint64_t> a_hashes, std::span<Handles::Sprite> handles) {
	CR_ASSERT(a_hashes.size() == handles.size(), "bad arguments");
	auto available = ~g_data->Used;
	cegraph::Handles::Sprite result{g_data->Used.FindNotInSet()};
	CR_ASSERT(available.size() >= a_hashes.size(), "ran out of Sprites");

	auto nextAvailable = available.begin();

	for(uint32_t i = 0; i < a_hashes.size(); ++i) {
		g_data->Used.insert(*nextAvailable);
		handles[i] = Handles::Sprite{*nextAvailable};

		auto spriteTemplate = g_data->TemplateLookup.find(a_hashes[i]);
		CR_ASSERT(spriteTemplate != g_data->TemplateLookup.end(), "Couldn't find sprite template");
		uint64_t textureHash = g_data->TemplateTextureHashes[spriteTemplate->second];

		auto textureHandle = cegraph::Textures::GetHandle(textureHash);
		CR_ASSERT(textureHandle.isValid(), "couldn't find requested texture for sprite");
		g_data->TextureHandles[*nextAvailable] = textureHandle;

		g_data->TemplateIndices[*nextAvailable] = spriteTemplate->second;
		g_data->NumFrames[*nextAvailable]       = ((uint16_t)cegraph::Textures::GetNumFrames(textureHandle) *
                                             cegraph::Constants::c_designRefreshRate) /
		                                    g_data->TemplateFrameRates[spriteTemplate->second];
		g_data->CurrentFrame[*nextAvailable] = 0;

		++nextAvailable;
	}
}

void cegraph::Sprites::DeleteInternal(std::span<Handles::Sprite> a_sprites) {
	for(auto sprite : a_sprites) {
		CR_ASSERT(g_data->Used.contains(sprite.asInt()), "tried to delete a sprite that doesn't exist");
		g_data->Used.erase(sprite.asInt());
	}
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
	g_data->TemplateFrameRates.reserve(sprites.size());
	for(uint16_t i = 0; i < sprites.size(); ++i) {
		g_data->TemplateNames.emplace_back(sprites[i]->name()->c_str());
		uint64_t hash = cecore::Hash64(sprites[i]->name()->c_str());
		g_data->TemplateHashes.emplace_back(hash);
		switch(sprites[i]->frame_rate()) {
			case cegraph::Flatbuffers::FrameRate::FPS60:
				g_data->TemplateFrameRates.emplace_back(60);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS30:
				g_data->TemplateFrameRates.emplace_back(30);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS20:
				g_data->TemplateFrameRates.emplace_back(20);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS15:
				g_data->TemplateFrameRates.emplace_back(15);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS12:
				g_data->TemplateFrameRates.emplace_back(12);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS10:
				g_data->TemplateFrameRates.emplace_back(10);
				break;
			default:
				CR_ASSERT(false, "Unknown sprite frame rate");
				g_data->TemplateFrameRates.emplace_back(1);
				break;
		}
		g_data->TemplateTextureHashes.emplace_back(cecore::Hash64(sprites[i]->texture()->c_str()));

		g_data->TemplateLookup[hash] = i;
	}
}

void cegraph::Sprites::Shutdown() {
	CR_ASSERT(g_data != nullptr, "Sprites are already shutdown");
	CR_ASSERT(g_data->Used.empty(), "Sprites weren't all freed before shutdown");

	delete g_data;
}

void cegraph::Sprites::SetPositionsInternal(std::span<Handles::Sprite> a_sprites,
                                            std::span<glm::vec2> a_positions) {
	CR_ASSERT(g_data != nullptr, "Sprites are shutdown");
	CR_ASSERT(a_sprites.size() == a_positions.size(), "Sprites SetPositionsInternal bad arguments");
	for(uint32_t i = 0; i < a_sprites.size(); ++i) {
		CR_ASSERT(g_data->Used.contains(a_sprites[i].asInt()), "Sprite doesn't exist");
	}
}

void cegraph::Sprites::SetRotationsInternal(std::span<Handles::Sprite> a_sprites,
                                            std::span<float> a_rotations) {
	CR_ASSERT(g_data != nullptr, "Sprites are shutdown");
	CR_ASSERT(a_sprites.size() == a_rotations.size(), "Sprites SetRotationsInternal bad arguments");
	for(uint32_t i = 0; i < a_sprites.size(); ++i) {
		CR_ASSERT(g_data->Used.contains(a_sprites[i].asInt()), "Sprite doesn't exist");
	}
}

void cegraph::Sprites::Update() {
	for(uint16_t sprite : g_data->Used) {
		g_data->CurrentFrame[sprite] += cegraph::GetContext().DisplayTicksPerFrame;
		if(g_data->CurrentFrame[sprite] > g_data->NumFrames[sprite]) {
			g_data->CurrentFrame[sprite] -= g_data->NumFrames[sprite];
		}
		uint32_t frame =
		    g_data->CurrentFrame[sprite] / (cegraph::Constants::c_designRefreshRate /
		                                    g_data->TemplateFrameRates[g_data->TemplateIndices[sprite]]);
	}
}