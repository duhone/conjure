#include <engine/Engine.h>

#include <cli11/cli11.hpp>

import CR.Engine;

import std;
import std.compat;

using namespace std::chrono_literals;
namespace fs  = std::filesystem;
namespace cec = CR::Engine::Core;
namespace cep = CR::Engine::Platform;

static constexpr const char* c_interfaceProto = R"(
export module CR.Tools.Embed.{0};

import <span.h>;

namespace CR::Tools::Embed{{
	export const std::span<const std::byte> Get{0}();
}}
)";

static constexpr const char* c_srcProtoBegin = R"(
module CR.Tools.Embed.{0};

const std::span<const std::byte> CR::Tools::Embed::Get{0}(){{
	static const std::byte data[] = {{)";

static constexpr const char* c_srcProtoEnd = R"(
	};

	return std::span<const std::byte>(data);
}
)";

int main(int argc, char** argv) {
	CLI::App app{"embed"};
	std::string inputFileName  = "";
	std::string outputFileName = "";
	app.add_option("-i,--input", inputFileName, "Input file to store in code")->required();
	app.add_option("-o,--output", outputFileName,
	               "Output file and path. filename only, both an .ixx and a .cpp will be generated at the "
	               "output location")
	    ->required();

	CLI11_PARSE(app, argc, argv);

	fs::path inputPath{inputFileName};
	fs::path outputPath{outputFileName};

	fs::current_path(cep::GetCurrentProcessPath());

	if(!fs::exists(inputPath)) {
		CLI::Error error{"input file", "Input file doesn't exist", CLI::ExitCodes::FileError};
		app.exit(error);
	}
	if(outputPath.has_extension()) {
		CLI::Error error{
		    "extension",
		    "do not add an extension to the output file name, .h and .cpp will be appended automaticly",
		    CLI::ExitCodes::FileError};
		app.exit(error);
	}

	fs::path outputInterface = outputPath.replace_extension(".ixx");
	fs::path outputSrc       = outputPath.replace_extension(".cpp");
	std::string varName      = outputPath.filename().replace_extension("").string();

	cep::MemoryMappedFile inputData(inputPath);

	bool needsUpdating = false;
	if(!fs::exists(outputInterface) ||
	   (fs::last_write_time(outputInterface) <= fs::last_write_time(inputPath))) {
		needsUpdating = true;
	}
	if(!fs::exists(outputSrc) || (fs::last_write_time(outputSrc) <= fs::last_write_time(inputPath))) {
		needsUpdating = true;
	}
	if(!needsUpdating) {
		return 0;    // nothing to do;
	}

	{
		fs::path outputFolder = outputInterface;
		outputFolder.remove_filename();
		if(!outputFolder.empty()) { fs::create_directories(outputFolder); }
	}

	{
		fs::path outputFolder = outputSrc;
		outputFolder.remove_filename();
		if(!outputFolder.empty()) { fs::create_directories(outputFolder); }
	}

	{
		std::string header = std::format(c_interfaceProto, varName);
		std::ofstream interfaceFile(outputInterface);
		interfaceFile << header;
	}
	{
		std::string protoBegin = std::format(c_srcProtoBegin, varName);
		std::ofstream srcFile(outputSrc);
		srcFile << protoBegin;

		for(size_t i = 0; i < inputData.size(); ++i) {
			if(i % 18 == 0) { srcFile << "\n\t\t"; }
			std::array<char, 10> str;
			auto [strEnd, err] = std::to_chars(data(str), data(str) + size(str), (uint8_t)inputData[i], 16);
			srcFile << "std::byte(0x" << std::string_view(data(str), strEnd - data(str)) << ')';
			if(i != inputData.size() - 1) { srcFile << ", "; }
		}

		srcFile << c_srcProtoEnd;
	}

	return 0;
}
