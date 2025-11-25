export module CR.Engine.Platform.Handles;

import CR.Engine.Core;

export namespace CR::Engine::Platform::Handles {
	export using File   = CR::Engine::Core::Handle<class FileTag>;
	export using Buffer = CR::Engine::Core::Handle<class BufferTag>;
}    // namespace CR::Engine::Platform::Handles
