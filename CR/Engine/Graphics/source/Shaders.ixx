module;

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
		Shaders(VkDevice a_device);
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

	  private:
		VkDevice m_device;

		std::unordered_map<uint64_t, VkShaderModule> m_shaderModules;
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

	ceplat::MemoryMappedFile CompileShader(const std::string_view a_srcPath, const fs::path& a_workingFolder,
	                                       const std::span<std::byte> a_data) {
		fs::path tempPathInput  = a_workingFolder / a_srcPath;
		fs::path tempPathOutput = tempPathInput;
		tempPathOutput += ".spirv";
		fs::create_directories(fs::path(tempPathInput).remove_filename());

		{
			cecore::FileHandle inputFile(tempPathInput);
			fwrite(a_data.data(), 1, a_data.size(), inputFile.asFile());
		}

		std::string cliArgs = fmt::format("{} -o {}", tempPathInput.string(), tempPathOutput.string());

		ceplat::Process glslc("glslc.exe", cliArgs.c_str());
		if(!glslc.WaitForClose(60s)) {
			CR_ERROR("glslc shader compiler did not complete after 60s");
			return {};
		}
		auto exitCode = glslc.GetExitCode();

		if(!exitCode.has_value() || exitCode.value() != 0) {
			CR_ERROR("failed to compile shader {}", a_srcPath);
			return {};
		}

		return ceplat::MemoryMappedFile(tempPathOutput);
	}
}    // namespace

cegraph::Shaders::Shaders(VkDevice a_device) : m_device(a_device) {
	auto& assetService = cecore::GetService<ceasset::Service>();

	fs::path workingFolder = fs::temp_directory_path() /= shaderCompileFolder;
	fs::create_directory(workingFolder);

	auto loadShader = [&](uint64_t a_hash, std::string_view a_path, const std::span<std::byte> a_data) {
		auto spirvFile = CompileShader(a_path, workingFolder, a_data);
		VkShaderModuleCreateInfo shaderInfo;
		ClearStruct(shaderInfo);
		shaderInfo.pCode    = (uint32_t*)spirvFile.data();
		shaderInfo.codeSize = spirvFile.size();

		VkShaderModule shaderModule;
		auto result = vkCreateShaderModule(m_device, &shaderInfo, nullptr, &shaderModule);
		CR_ASSERT(result == VK_SUCCESS, "failed to load shader {}", a_path);
		m_shaderModules.emplace(a_hash, shaderModule);
	};

	assetService.Load(ceasset::Service::Partitions::Graphics, "Shaders", "comp", loadShader);
	assetService.Load(ceasset::Service::Partitions::Graphics, "Shaders", "vert", loadShader);
	assetService.Load(ceasset::Service::Partitions::Graphics, "Shaders", "frag", loadShader);

	fs::remove_all(workingFolder);
}

cegraph::Shaders::~Shaders() {
	for(const auto& shader : m_shaderModules) { vkDestroyShaderModule(m_device, shader.second, nullptr); }
}
