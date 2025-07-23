export module CR.Engine.Compression.General;

import CR.Engine.Core;

import std;
import std.compat;

namespace CR::Engine::Compression::General {
	// a_level is -5 to 22. 3 is a good default if you need fairly fast compression and decompression.
	// 18 is best if you need fast decompression, but dont care about compression speed. -5 if you need
	// fastest possible.
	// Note that for perf reasons the returned buffers may use a lot more memory than their reported size.
	// Just shrinkToFit the returned value if needed.
	export CR::Engine::Core::Buffer Compress(const std::span<const std::byte> a_src, int32_t a_level);
	export CR::Engine::Core::Buffer Decompress(const std::span<const std::byte> a_src);
}    // namespace CR::Engine::Compression::General
