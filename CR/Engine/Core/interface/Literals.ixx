export module CR.Engine.Core.Literals;

export namespace CR::Engine::Core::Literals {
	constexpr unsigned long long operator"" _KB(unsigned long long a_bytes) {
		return a_bytes * 1024;
	}
	constexpr unsigned long long operator"" _MB(unsigned long long a_bytes) {
		return a_bytes * 1024 * 1024;
	}
	constexpr unsigned long long operator"" _GB(unsigned long long a_bytes) {
		return a_bytes * 1024 * 1024 * 1024;
	}
}    // namespace CR::Engine::Core::Literals
