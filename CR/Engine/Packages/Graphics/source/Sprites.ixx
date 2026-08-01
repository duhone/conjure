module;

#include "generated/graphics/sprites_generated.h"

#include "flatbuffers/idl.h"

#include <core/Log.h>

#include "Core.h"

#include "ankerl/unordered_dense.h"
#include <glm/glm.hpp>

export module CR.Engine.Graphics.Sprites;

import CR.Engine.Graphics.Constants;
import CR.Engine.Graphics.Context;
import CR.Engine.Graphics.Handles;
import CR.Engine.Graphics.Materials;
import CR.Engine.Graphics.MultiDrawBuffer;
import CR.Engine.Graphics.Textures;
import CR.Engine.Graphics.VertexLayout;
import CR.Engine.Graphics.InternalHandles;
import CR.Engine.Graphics.VertexBuffers;

import CR.Engine.Assets;
import CR.Engine.Core;

import std;
import std.compat;

export namespace CR::Engine::Graphics::Sprites {
	extern "C++" void Create(std::span<uint64_t> a_hashes, std::span<Handles::Sprite> handles);
	extern "C++" void Delete(std::span<Handles::Sprite> a_sprites);
	extern "C++" void SetPositions(std::span<Handles::Sprite> a_sprites, std::span<glm::vec2> a_positions);
	extern "C++" void SetRotations(std::span<Handles::Sprite> a_sprites, std::span<float> a_rotations);

	void Initialize();
	void Shutdown();
	void Update();
	void Render(VkCommandBuffer& a_cmdBuffer);

}    // namespace CR::Engine::Graphics::Sprites

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;

namespace {
#pragma pack(push)
#pragma pack(1)
	struct Vertex {
		glm::vec2 Offset;
		glm::u16vec2 TextureFrame;
		glm::u8vec4 Color;
		glm::u16vec2 FrameSize;
		glm::mat2x2 Rotation;
	};
#pragma pack(pop)

	cecore::HandlePool<cegraph::Handles::Sprite, cegraph::Constants::c_maxSprites> m_handlePool;
	std::array<cegraph::Handles::Texture, cegraph::Constants::c_maxSprites> m_textureHandles;
	std::array<glm::vec2, cegraph::Constants::c_maxSprites> m_positions;
	std::array<float, cegraph::Constants::c_maxSprites> m_rotations;
	// these aren't the display frame. its the display frame * fps divisor.
	std::array<uint32_t, cegraph::Constants::c_maxSprites> m_numFrames;
	std::array<uint32_t, cegraph::Constants::c_maxSprites> m_currentFrames;
	// actual display frames. i.e. the traditional sprite frame
	std::array<uint32_t, cegraph::Constants::c_maxSprites> m_displayFrames;
	std::array<glm::uvec2, cegraph::Constants::c_maxSprites> m_dimensions;
	std::array<uint16_t, cegraph::Constants::c_maxSprites> m_templateIndices;

	// templates
	std::vector<std::string> m_templateNames;
	std::vector<uint64_t> m_templateHashes;
	std::vector<uint8_t> m_templateFrameRates;
	std::vector<uint64_t> m_templateTextureHashes;
	ankerl::unordered_dense::map<uint64_t, uint16_t> m_templateLookup;

	// GPU
	cegraph::Handles::VertexBuffer m_vertBuffer;
	cegraph::Handles::Material m_material;
}    // namespace

void cegraph::Sprites::Create(std::span<uint64_t> a_hashes, std::span<Handles::Sprite> handles) {
	CR_ASSERT(a_hashes.size() == handles.size(), "bad arguments");
	CR_ASSERT(m_handlePool.available() >= a_hashes.size(), "ran out of Sprites");

	m_handlePool.acquire(handles);

	for(uint32_t i = 0; i < a_hashes.size(); ++i) {
		auto spriteTemplate = m_templateLookup.find(a_hashes[i]);
		CR_ASSERT(spriteTemplate != m_templateLookup.end(), "Couldn't find sprite template");
		uint64_t textureHash = m_templateTextureHashes[spriteTemplate->second];

		auto textureHandle = cegraph::Textures::GetHandle(textureHash);
		CR_ASSERT(textureHandle.isValid(), "couldn't find requested texture for sprite");
		m_textureHandles[handles[i]] = textureHandle;

		m_templateIndices[handles[i]] = spriteTemplate->second;
		m_numFrames[handles[i]]       = ((uint16_t)cegraph::Textures::GetNumFrames(textureHandle) *
		                                 cegraph::Constants::c_designRefreshRate) /
		                                m_templateFrameRates[spriteTemplate->second];
		m_currentFrames[handles[i]]   = 0;
		m_dimensions[handles[i]]      = Textures::GetDimensions(textureHandle);
	}
}

void cegraph::Sprites::Delete(std::span<Handles::Sprite> a_sprites) {
	m_handlePool.release(a_sprites);
}

void cegraph::Sprites::Initialize() {
	m_material = cegraph::Materials::GetMaterial("sprite");
	CR_ASSERT(m_material.isValid(), "Failed to get sprite material");

	flatbuffers::Parser parser = ceasset::GetData(cecore::C_Hash64("Graphics/sprites.json"), SCHEMAS_SPRITES);
	auto spritesfb             = Flatbuffers::GetSprites(parser.builder_.GetBufferPointer());

	const auto& sprites = *spritesfb->sprites();

	m_templateNames.reserve(sprites.size());
	m_templateHashes.reserve(sprites.size());
	m_templateFrameRates.reserve(sprites.size());
	for(uint16_t i = 0; i < sprites.size(); ++i) {
		m_templateNames.emplace_back(sprites[i]->name()->c_str());
		uint64_t hash = cecore::Hash64(sprites[i]->name()->c_str());
		m_templateHashes.emplace_back(hash);
		switch(sprites[i]->frame_rate()) {
			case cegraph::Flatbuffers::FrameRate::FPS60:
				m_templateFrameRates.emplace_back(60);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS30:
				m_templateFrameRates.emplace_back(30);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS20:
				m_templateFrameRates.emplace_back(20);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS15:
				m_templateFrameRates.emplace_back(15);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS12:
				m_templateFrameRates.emplace_back(12);
				break;
			case cegraph::Flatbuffers::FrameRate::FPS10:
				m_templateFrameRates.emplace_back(10);
				break;
			default:
				CR_ASSERT(false, "Unknown sprite frame rate");
				m_templateFrameRates.emplace_back(1);
				break;
		}
		m_templateTextureHashes.emplace_back(cecore::Hash64(sprites[i]->texture()->c_str()));

		m_templateLookup[hash] = i;
	}

	Vertex dummy;
	cegraph::VertexLayout vertLayout;
	vertLayout.AddVariable(dummy.Offset);
	vertLayout.AddVariable(dummy.TextureFrame);
	vertLayout.AddVariable(dummy.Color);
	vertLayout.AddVariable(dummy.FrameSize);
	vertLayout.AddVariable(dummy.Rotation);
	m_vertBuffer = VertexBuffers::Create(vertLayout, Constants::c_maxSprites);
}

void cegraph::Sprites::Shutdown() {
	CR_ASSERT(m_handlePool.full(), "Sprites weren't all freed before shutdown");

	VertexBuffers::Release(m_vertBuffer);
}

void cegraph::Sprites::SetPositions(std::span<Handles::Sprite> a_sprites, std::span<glm::vec2> a_positions) {
	CR_ASSERT(a_sprites.size() == a_positions.size(), "Sprites SetPositions bad arguments");
	for(uint32_t i = 0; i < a_sprites.size(); ++i) {
		CR_ASSERT(m_handlePool.isValid(a_sprites[i]), "Sprite doesn't exist");
		m_positions[a_sprites[i]] = a_positions[i];
	}
}

void cegraph::Sprites::SetRotations(std::span<Handles::Sprite> a_sprites, std::span<float> a_rotations) {
	CR_ASSERT(a_sprites.size() == a_rotations.size(), "Sprites SetRotations bad arguments");
	for(uint32_t i = 0; i < a_sprites.size(); ++i) {
		CR_ASSERT(m_handlePool.isValid(a_sprites[i]), "Sprite doesn't exist");
		m_rotations[a_sprites[i]] = a_rotations[i];
	}
}

void cegraph::Sprites::Update() {
	auto mapping        = VertexBuffers::Map(m_vertBuffer);
	Vertex* spriteProps = (Vertex*)mapping.Data;

	for(uint16_t sprite : m_handlePool) {
		m_currentFrames[sprite] += cegraph::GetContext().DisplayTicksPerFrame;
		if(m_currentFrames[sprite] > m_numFrames[sprite]) { m_currentFrames[sprite] -= m_numFrames[sprite]; }
		m_displayFrames[sprite] = m_currentFrames[sprite] / (cegraph::Constants::c_designRefreshRate /
		                                                     m_templateFrameRates[m_templateIndices[sprite]]);

		float sinAngle = sin(m_rotations[sprite]);
		float cosAngle = cos(m_rotations[sprite]);

		glm::mat2 rot = glm::mat2{cosAngle, -sinAngle, sinAngle, cosAngle};

		spriteProps->Offset       = m_positions[sprite];
		spriteProps->TextureFrame = {m_textureHandles[sprite], m_displayFrames[sprite]};
		spriteProps->Color        = glm::u8vec4{255, 255, 255, 255};
		spriteProps->FrameSize    = m_dimensions[sprite];
		spriteProps->Rotation     = rot;

		++spriteProps;
	}
}

void cegraph::Sprites::Render(VkCommandBuffer& a_cmdBuffer) {
	Materials::Bind(m_material, a_cmdBuffer);
	VertexBuffers::Bind(m_vertBuffer, a_cmdBuffer);

	auto spriteCount = m_handlePool.used();

	auto drawMap  = MultiDrawBuffer::Map(spriteCount);
	auto commands = drawMap.Data;
	for(uint32_t i = 0; i < spriteCount; ++i) {
		commands->vertexCount   = 4;
		commands->instanceCount = 1;
		commands->firstVertex   = 0;
		commands->firstInstance = i;
		++commands;
	}
	vkCmdDrawIndirect(a_cmdBuffer, drawMap.Buffer, 0, spriteCount, sizeof(VkDrawIndirectCommand));
}
