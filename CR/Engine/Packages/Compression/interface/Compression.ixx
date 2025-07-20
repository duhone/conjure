export module CR.Engine.Compression;

export import CR.Engine.Compression.General;
export import CR.Engine.Compression.Wav;

export namespace CR::Engine::Compression {
	void Initialize();
	void Update();
	void Render();
	void Shutdown();
}    // namespace CR::Engine::Compression

module :private;

namespace cecomp = CR::Engine::Compression;

// comp doesn't really have any systems. not much to do
void cecomp::Initialize() {}
void cecomp::Update() {}
void cecomp::Render() {}
void cecomp::Shutdown() {}
