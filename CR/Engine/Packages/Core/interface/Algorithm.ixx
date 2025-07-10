export module CR.Engine.Core.Algorithm;

import std;

export namespace CR::Engine::Core {
	void UnorderedRemove(std::ranges::range auto& a_container, std::input_iterator auto& a_iterator) {
		*a_iterator = *a_container.back();
		a_container.pop_back();
	}

	std::size_t SortedInsert(std::ranges::range auto& a_container, const std::regular auto& a_value,
	                         std::strict_weak_order auto a_compare) {
		auto it = std::upper_bound(begin(a_container), end(a_container), a_value, a_compare);
		it      = a_container.insert(it, a_value);
		return std::distance(a_container.begin(), it);
	}

	template<typename... Ts>
	void for_each_argument(std::invocable<Ts...> auto&& a_callable, Ts&&... a_args) {
		(void)std::initializer_list<int>{(a_callable(std::forward<Ts>(a_args)), 0)...};
	}
}    // namespace CR::Engine::Core
