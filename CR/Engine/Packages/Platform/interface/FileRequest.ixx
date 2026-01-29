export module CR.Engine.Platform.FileRequest;

import CR.Engine.Platform.Handles;

import std;
import std.compat;

export namespace CR::Engine::Platform::FileRequest {

	// ideally in packed mode, only 1, or at most a handful of files. just register them once at app start.
	// In loose mode, just register while reading, will run out otherwise.
	Handles::File registerFile(const std::filesystem::path& a_filePath);
	void unregisterFile(Handles::File a_file);
	Handles::Buffer registerBuffer(std::span<std::byte> a_buffer);
	void unregisterBuffer(Handles::Buffer a_buffer);

	// completed and buffer must outlive the load
	struct LoadArgs {
		const std::filesystem::path& filePath;
		std::atomic_bool& completed;
		int32_t fileOffset;
		std::span<std::byte> buffer;
	};
	void Load(const LoadArgs& a_args);

	export namespace Internal {
		void initialize();
		void update();
		void shutdown();
	}    // namespace Internal
}    // namespace CR::Engine::Platform::FileRequest
