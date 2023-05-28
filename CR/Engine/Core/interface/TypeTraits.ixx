export module CR.Engine.Core.TypeTraits;

import <type_traits>;

export namespace CR::Engine::Core {
	template<typename T, typename... Rest>
	struct is_one_of;

	template<typename T>
	struct is_one_of<T> : std::false_type {};

	template<typename T, typename... Rest>
	struct is_one_of<T, T, Rest...> : std::true_type {};

	template<typename T, typename First, typename... Rest>
	struct is_one_of<T, First, Rest...> : is_one_of<T, Rest...> {};

	template<typename... Rest>
	using is_one_of_t = typename is_one_of<Rest...>::type;

	template<typename... Rest>
	inline constexpr bool is_one_of_v = is_one_of<Rest...>::value;

	template<class T>
	struct always_false : std::false_type {};

	template<class T>
	inline constexpr bool always_false_v = always_false<T>::value;

	template<class T>
	struct always_true : std::true_type {};

	template<class T>
	inline constexpr bool always_true_v = always_true<T>::value;

	template<typename... Rest>
	struct is_unique;

	template<class T>
	struct is_unique<T> : std::true_type {};

	template<class First, class Second>
	struct is_unique<First, Second> : std::negation<std::is_same<First, Second>> {};

	template<class First, class Second, class... Rest>
	struct is_unique<First, Second, Rest...>
	    : std::conjunction<is_unique<First, Second>, is_unique<First, Rest...>, is_unique<Second, Rest...>> {
	};

	template<typename... Rest>
	using is_unique_t = typename is_unique<Rest...>::type;

	template<typename... Rest>
	inline constexpr bool is_unique_v = is_unique<Rest...>::value;

	// converts std:function style definition to a function pointer type. i.e. from
	// something like int(float) to int(*)(float)
	template<typename T>
	struct GetFunctionPtrType;

	template<typename RET, typename... Args>
	struct GetFunctionPtrType<RET(Args...)> {
		using type = RET (*)(Args...);
	};

	template<typename T>
	using GetFunctionPtrType_t = typename GetFunctionPtrType<T>::type;
}    // namespace CR::Engine::Core
