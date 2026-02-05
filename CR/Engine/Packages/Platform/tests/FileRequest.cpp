import CR.Engine.Platform.FileRequest;
import CR.Engine.Platform.PathUtils;

// import std;

#include <doctest/doctest.h>

namespace cep = CR::Engine::Platform;

namespace fs = std::filesystem;

TEST_CASE("FileRequest") {
	cep::FileRequest::Internal::initialize();

	auto testFilePath = cep::GetCurrentProcessPath();
	testFilePath.append("test.txt");
	auto bufsize = fs::file_size(testFilePath);

	auto buffer = std::make_unique_for_overwrite<std::byte[]>(bufsize);

	auto fileHandle   = cep::FileRequest::registerFile(testFilePath);
	auto bufferHandle = cep::FileRequest::registerBuffer({buffer.get(), bufsize});

	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	/*cep::MemoryMappedFile mmapFile{testFilePath.c_str()};
	REQUIRE(mmapFile.size() == 2);
	auto data = mmapFile.data();
	REQUIRE((char)data[0] == '5');
	REQUIRE((char)data[1] == '8');

	cep::MemoryMappedFile mmapFile2 = std::move(mmapFile);
	REQUIRE(mmapFile.size() == 0);
	REQUIRE(mmapFile2.size() == 2);*/

	cep::FileRequest::unregisterBuffer(bufferHandle);
	cep::FileRequest::unregisterFile(fileHandle);

	cep::FileRequest::Internal::shutdown();
}
