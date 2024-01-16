module;

#include "generated/graphics/shaders_generated.h"

#include "flatbuffers/idl.h"

#include <function2/function2.hpp>

#include "core/Log.h"

#include "Core.h"

#include <fmt/format.h>

export module CR.Engine.Graphics.Shaders;

import CR.Engine.Graphics.GraphicsThread;
import CR.Engine.Graphics.Utils;

import CR.Engine.Assets;
import CR.Engine.Core;
import CR.Engine.Platform;

import <chrono>;
import <filesystem>;
import <span>;
import <string_view>;
import <unordered_map>;

namespace CR::Engine::Graphics {
	export class Shaders {
	  public:
		Shaders() = default;
		Shaders(VkDevice a_device, GraphicsThread& a_thread);
		~Shaders();
		Shaders(const Shaders&)               = delete;
		Shaders(Shaders&& a_other)            = delete;
		Shaders& operator=(const Shaders&)    = delete;
		Shaders& operator=(Shaders&& a_other) = delete;

		VkShaderModule GetShader(uint64_t a_shaderHash) const {
			auto shader = m_shaderModules.find(a_shaderHash);
			if(shader != m_shaderModules.end()) { return shader->second; }
			CR_ERROR("Could not find shader");
			return nullptr;
		}

		bool IsReady() const { return m_ready.load(std::memory_order_acquire); }

	  private:
		VkDevice m_device;

		std::unordered_map<uint64_t, VkShaderModule> m_shaderModules;

		std::atomic_bool m_ready;
	};
}    // namespace CR::Engine::Graphics

module :private;

namespace ceasset = CR::Engine::Assets;
namespace cecore  = CR::Engine::Core;
namespace cegraph = CR::Engine::Graphics;
namespace ceplat  = CR::Engine::Platform;

namespace fs = std::filesystem;

using namespace std::chrono_literals;
using namespace std::literals;

namespace {
	constexpr std::string_view shaderCompileFolder = "conjure_shaders"sv;

	ceplat::MemoryMappedFile CompileShader(const fs::path a_srcPath, const fs::path& a_workingFile) {
		std::string cliArgs = fmt::format("{} -o {}", a_srcPath.string(), a_workingFile.string());

		ceplat::Process glslc("glslc.exe", cliArgs.c_str());
		if(!glslc.WaitForClose(60s)) {
			CR_ERROR("glslc shader compiler did not complete after 60s");
			return {};
		}
		auto exitCode = glslc.GetExitCode();

		if(!exitCode.has_value() || exitCode.value() != 0) {
			CR_ERROR("failed to compile shader {}", a_srcPath.string());
			return {};
		}

		return ceplat::MemoryMappedFile(a_workingFile);
	}
}    // namespace

cegraph::Shaders::Shaders(VkDevice a_device, GraphicsThread& a_thread) : m_device(a_device) {
	a_thread.EnqueueTask(
	    [this]() {
		    auto& assetService   = cecore::GetService<ceasset::Service>();
		    const auto& rootPath = assetService.GetRootPath();

		    fs::path workingFile = fs::temp_directory_path() / "conjure_compiled_shader.spirv";

		    flatbuffers::Parser parser;
		    ceplat::MemoryMappedFile schemaFile(SCHEMAS_SHADERS);
		    std::string schemaData((const char*)schemaFile.data(), schemaFile.size());
		    parser.Parse(schemaData.c_str());

		    auto shadersData = assetService.GetData(cecore::C_Hash64("Graphics/shaders.json"));
		    std::string flatbufferJson((const char*)shadersData.data(), shadersData.size());
		    parser.ParseJson(flatbufferJson.c_str());
		    CR_ASSERT(parser.BytesConsumed() <= (ptrdiff_t)shadersData.size(),
		              "buffer overrun loading shaders.json");
		    auto shaders = Flatbuffers::GetShaders(parser.builder_.GetBufferPointer());

		    for(const auto& shader : *shaders->shaders()) {
			    auto spirvFile = CompileShader(rootPath / shader->path()->string_view(), workingFile);
			    VkShaderModuleCreateInfo shaderInfo;
			    ClearStruct(shaderInfo);
			    shaderInfo.pCode    = (uint32_t*)spirvFile.data();
			    shaderInfo.codeSize = spirvFile.size();

			    VkShaderModule shaderModule;
			    auto result = vkCreateShaderModule(m_device, &shaderInfo, nullptr, &shaderModule);
			    CR_ASSERT(result == VK_SUCCESS, "failed to load shader {}", shader->path()->string_view());
			    m_shaderModules.emplace(cecore::Hash64(shader->name()->string_view()), shaderModule);
		    }

		    fs::remove(workingFile);
	    },
	    m_ready);
}

cegraph::Shaders::~Shaders() {
	for(const auto& shader : m_shaderModules) { vkDestroyShaderModule(m_device, shader.second, nullptr); }
}
