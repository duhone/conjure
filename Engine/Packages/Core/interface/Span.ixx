export module CR.Engine.Core.Span;

import std;

export namespace CR::Engine::Core {
	// For use as a function *parameter* type on APIs that take either one element or many, so a single
	// function handles both:
	//   void SetColors(SpanArg<const Sprite> a_sprites, SpanArg<const Color> a_colors);
	// Accepts anything std::span accepts (vectors, arrays, spans, iterator pairs...) plus a single element.
	// With a const T a temporary single element is fine too. It is a std::span, so it passes on to
	// anything wanting a std::span<T> / std::span<const T> without casting.
	//
	// Only use as a parameter, never store it, the single element constructor refers to the caller's
	// object. A braced list can't be used for a single element, name the type: SetPositions(s, glm::vec2{1, 2})
	template<typename T>
	class SpanArg : public std::span<T> {
	  public:
		// all of span's constructors (ranges, pointer + count, arrays, iterator pairs, etc.)
		using std::span<T>::span;

		// span's copy constructor isn't inherited, so need this to construct from a std::span.
		constexpr SpanArg(std::span<T> a_span) noexcept : std::span<T>(a_span) {}

		constexpr SpanArg(T& a_element) noexcept : std::span<T>(std::addressof(a_element), 1) {}
	};
}    // namespace CR::Engine::Core
