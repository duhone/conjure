export module CR.Engine;

export import CR.Engine.Core;
// export import CR.Engine.Platform;

export namespace CR::Engine {
	void Initialize();
	void Update();
	void Render();
	void Shutdown();
}    // namespace CR::Engine

module :private;

namespace cecore = CR::Engine::Core;

void CR::Engine::Initialize() {
	cecore::Initialize();
}

void CR::Engine::Update() {
	cecore::Update();
}

void CR::Engine::Render() {
	cecore::Update();
}

void CR::Engine::Shutdown() {
	cecore::Shutdown();
}
