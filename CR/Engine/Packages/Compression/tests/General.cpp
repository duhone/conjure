import CR.Engine.Compression;
import CR.Engine.Platform;
import CR.Engine.Core;

import std;

#include <doctest/doctest.h>

namespace cecore = CR::Engine::Core;
namespace cecomp = CR::Engine::Compression;
namespace ceplat = CR::Engine::Platform;

TEST_CASE("basic compression") {
	auto test = [](const char* testFile) {
		printf("%s test\n", testFile);
		auto testFilePath = cep::GetCurrentProcessPath() / testFile;
		cep::MemoryMappedFile testFileMMap(testFilePath.c_str());

		std::unique_ptr<std::byte[]> compressedData;
		std::unique_ptr<std::byte[]> decompressedData;

		auto test1 = [&](const char* label, int32_t level) {
			printf("%s mode\n", label);
			{
				cec::ScopedTimer time("compress time");
				compressedData = cecomp::General::Compress(
				    std::span<const std::byte>(testFileMMap.data(), (uint32_t)testFileMMap.size()), level);
			}
			{
				cec::ScopedTimer time("decompress time");
				decompressedData = cecomp::General::Decompress(
				    std::span<const std::byte>(compressedData.data(), (uint32_t)compressedData.size()));
			}
			REQUIRE(decompressedData.size() == testFileMMap.size());
			REQUIRE(memcmp(data(decompressedData), testFileMMap.data(), size(compressedData)) == 0);
			printf("compression ration %0.2f\n\n", ((float)decompressedData.size() / compressedData.size()));
		};

		test1("level -5", -5);
		// test1("level -1", -1);
		// test1("level 1", 1);
		test1("level 3", 3);
		// test1("level 6", 6);
		test1("level 9", 9);
		// test1("level 12", 12);
		// test1("level 15", 15);
		test1("level 18", 18);
		// test1("level 22", 22);
	};

	const char* testFiles[] = {"./alice29.txt",         "./cp.html",          "./data.xml",   "./game.cpp",
	                           "./TitleThemeRemix.wav", "./bumble_tales.tga", "./kodim23.tga"};

	for(const auto& testFile : testFiles) { test(testFile); }
}
