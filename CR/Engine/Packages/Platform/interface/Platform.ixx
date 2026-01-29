export module CR.Engine.Platform;

export import CR.Engine.Platform.FileRequest;
export import CR.Engine.Platform.Guid;
export import CR.Engine.Platform.MemoryMappedFile;
export import CR.Engine.Platform.PathUtils;
export import CR.Engine.Platform.Process;
export import CR.Engine.Platform.SharedLibrary;

export namespace CR::Engine::Platform { namespace Internal {

	void Initialize();
	void Update();
	void Shutdown();
}}    // namespace CR::Engine::Platform::Internal

module :private;

namespace ceplat = CR::Engine::Platform;

void ceplat::Internal::Initialize() {
	FileRequest::Internal::initialize();
}

void ceplat::Internal::Update() {
	FileRequest::Internal::update();
}

void ceplat::Internal::Shutdown() {
	FileRequest::Internal::shutdown();
}