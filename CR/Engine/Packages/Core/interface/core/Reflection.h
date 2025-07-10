#pragma once
import std;

namespace {
#define ReflectMember(varName)                                                                               \
	template<typename, typename = void>                                                                      \
	struct HasMember##varName : std::false_type {};                                                          \
	template<typename T>                                                                                     \
	struct HasMember##varName<T, std::void_t<decltype(std::declval<T>().varName)>> : std::true_type {};      \
	template<typename T>                                                                                     \
	using HasMember##varName##_t = typename HasMember##varName<T>::type;                                     \
	template<typename T>                                                                                     \
	constexpr bool HasMember##varName##_v = HasMember##varName##_t<T>::value;

// These templates require you to pass the types of the arguments too.
#define ReflectFuncion(funcName)                                                                             \
	template<typename, typename Void, typename... Args>                                                      \
	struct HasFuncion##funcName : std::false_type {};                                                        \
	template<typename T, typename... Args>                                                                   \
	struct HasFuncion##funcName<                                                                             \
	    T, std::void_t<decltype(std::declval<T>().funcName(std::declval<Args>()...))>, Args...>              \
	    : std::true_type {};                                                                                 \
	template<typename T, typename... Args>                                                                   \
	using HasFuncion##funcName##_t = typename HasFuncion##funcName<T, void, Args...>::type;                  \
	template<typename T, typename... Args>                                                                   \
	constexpr bool HasFuncion##funcName##_v = HasFuncion##funcName##_t<T, Args...>::value;
}    // namespace