export module CR.Engine.Platform.FileRequest;

import CR.Engine.Platform.Handles;

import std;
import std.compat;

// FileRequest is async. However all processing will happen in the order requested. i.e you could call load
// and immediatly call unregister for some reason, and it will be fine. the load will complete before the
// unregister is handled.
// Not really used yet, and not complete. even API prob needs changing. Mostly wanted to play with ioring.
export namespace CR::Engine::Platform::FileRequest {

	// reading from a registered file may be a little faster. Should not have many of these though. Probably 1
	// if using packed assets.
	Handles::File registerFile(const std::filesystem::path& a_filePath);
	void unregisterFile(Handles::File a_file);
	Handles::Buffer registerBuffer(std::span<std::byte> a_buffer);
	void unregisterBuffer(Handles::Buffer a_buffer);

	struct ReadArgs {
		Handles::File fileHandle{};
		Handles::Buffer bufferHandle{};
		uint32_t fileOffset{};
		uint32_t bufferOffset{};
		uint32_t readSize{};
	};
	Handles::Read read(const ReadArgs& a_args);
	// this will return true one time, after that the handle is no longer valid
	bool hasReadCompleted(Handles::Read a_handle);

	std::span<std::byte> map(Handles::Buffer a_buffer);

	export namespace Internal {
		void initialize();
		void update();
		void shutdown();
	}    // namespace Internal
}    // namespace CR::Engine::Platform::FileRequest
