export module CR.Engine.Core.Defer;

import std;

export namespace CR::Engine::Core {
	template<std::invocable<> callable_t>
	struct Defer {
		explicit Defer(callable_t&& f) noexcept : onExit(std::move(f)) {}
		~Defer() { onExit(); }

		Defer(Defer const&)          = delete;
		void operator=(Defer const&) = delete;
		Defer& operator=(Defer&&)    = delete;
		Defer(Defer&& rhs)           = delete;

	  private:
		callable_t onExit;
	};
}    // namespace CR::Engine::Core