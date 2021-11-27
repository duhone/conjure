export module CR.Engine.Core.Hash;

import<cstddef>;
import<span>;

export namespace CR::Engine::Core {
	// Paul Hsieh hash
	uint32_t HashFast(std::span<std::byte> a_data);
}    // namespace CR::Core