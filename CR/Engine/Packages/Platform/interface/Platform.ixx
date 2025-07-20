export module CR.Engine.Platform;

export import CR.Engine.Platform.Guid;
export import CR.Engine.Platform.MemoryMappedFile;
export import CR.Engine.Platform.PathUtils;
export import CR.Engine.Platform.PipeClient;
export import CR.Engine.Platform.PipeServer;
export import CR.Engine.Platform.Process;
export import CR.Engine.Platform.SharedLibrary;
export import CR.Engine.Platform.SharedMemory;
export import CR.Engine.Platform.Window;

export namespace CR::Engine::Platform {
	void Initialize();
	void Update();
	void Render();
	void Shutdown();
}    // namespace CR::Engine::Platform

module :private;

namespace ceplat = CR::Engine::Platform;

// platform doesn't really have any systems. not much to do
void ceplat::Initialize() {}
void ceplat::Update() {}
void ceplat::Render() {}
void ceplat::Shutdown() {}
