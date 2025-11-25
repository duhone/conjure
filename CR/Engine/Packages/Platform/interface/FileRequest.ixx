export module CR.Engine.Platform.FileRequest;

import CR.Engine.Platform.Handles;

import std;
import std.compat;

export namespace CR::Engine::Platform::FileRequest {

	Handles::File RegisterFile(const std::filesystem::path& a_filePath);
	void UnregisterFile(Handles::File a_file);

	// completed and buffer must outlive the load
	struct LoadArgs {
		const std::filesystem::path& filePath;
		std::atomic_bool& completed;
		int32_t fileOffset;
		std::span<std::byte> buffer;
	};
	void Load(const LoadArgs& a_args);

	export namespace Internal {
		void Initialize();
		void Update();
		void Shutdown();
	}    // namespace Internal
}    // namespace CR::Engine::Platform::FileRequest
