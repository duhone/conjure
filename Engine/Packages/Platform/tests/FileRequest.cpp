import CR.Engine.Platform.FileRequest;
import CR.Engine.Platform.PathUtils;

#include <doctest/doctest.h>

import std;

namespace cep = CR::Engine::Platform;

namespace fs = std::filesystem;

using namespace std::chrono_literals;

TEST_CASE("FileRequest") {
	cep::FileRequest::Internal::initialize();

	auto testFilePath = cep::GetCurrentProcessPath();
	testFilePath.append("test.txt");
	auto bufsize = fs::file_size(testFilePath);
	REQUIRE(bufsize == 2);

	auto buffer = std::make_unique_for_overwrite<std::byte[]>(4096);

	auto fileHandle   = cep::FileRequest::registerFile(testFilePath);
	auto bufferHandle = cep::FileRequest::registerBuffer({buffer.get(), 4096});

	cep::FileRequest::ReadArgs readArgs;
	readArgs.fileHandle   = fileHandle;
	readArgs.fileOffset   = 0;
	readArgs.bufferHandle = bufferHandle;
	readArgs.bufferOffset = 0;
	readArgs.readSize     = (uint32_t)4096;

	auto readHandle = cep::FileRequest::read(readArgs);

	while(!cep::FileRequest::hasReadCompleted(readHandle)) { std::this_thread::sleep_for(16ms); }

	auto data = buffer.get();
	REQUIRE((char)data[0] == '5');
	REQUIRE((char)data[1] == '8');

	cep::FileRequest::unregisterBuffer(bufferHandle);
	cep::FileRequest::unregisterFile(fileHandle);

	cep::FileRequest::Internal::shutdown();
}
