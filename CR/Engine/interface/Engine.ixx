export module CR.Engine;

export import CR.Engine.Core;
export import CR.Engine.Platform;
export import CR.Engine.Compression;

export namespace CR::Engine {
	void Initialize();
	void Update();
	void Render();
	void Shutdown();
}    // namespace CR::Engine

module :private;

namespace cecore = CR::Engine::Core;
namespace ceplat = CR::Engine::Platform;
namespace cecomp = CR::Engine::Compression;

void CR::Engine::Initialize() {
	cecore::Initialize();
	ceplat::Initialize();
	cecomp::Initialize();
}

void CR::Engine::Update() {
	cecore::Update();
	ceplat::Update();
	cecomp::Update();
}

void CR::Engine::Render() {
	cecore::Render();
	ceplat::Render();
	cecomp::Render();
}

void CR::Engine::Shutdown() {
	cecore::Shutdown();
	ceplat::Shutdown();
	cecomp::Shutdown();
}
