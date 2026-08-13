import CR.Engine.Platform.MemoryMappedFile;
import CR.Engine.Platform.PathUtils;

#include <doctest/doctest.h>

namespace cep = CR::Engine::Platform;

TEST_CASE("memory mapped files") {
	auto testFilePath = cep::GetCurrentProcessPath();
	testFilePath.append("test.txt");
	cep::MemoryMappedFile mmapFile{testFilePath.c_str()};
	REQUIRE(mmapFile.size() == 2);
	auto data = mmapFile.data();
	REQUIRE((char)data[0] == '5');
	REQUIRE((char)data[1] == '8');

	cep::MemoryMappedFile mmapFile2 = std::move(mmapFile);
	REQUIRE(mmapFile.size() == 0);
	REQUIRE(mmapFile2.size() == 2);
}
