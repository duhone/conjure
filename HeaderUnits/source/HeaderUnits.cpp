// std header units, get rid of these and use import std in actual code files instead, once on C++23. except a few that
// will need to stay here, ones that export macros. like cassert.
import <algorithm>;
import <any>;
import <array>;
import <bit>;
import <cassert>;
import <charconv>;
import <chrono>;
import <cmath>;
import <compare>;
import <concepts>;
import <cstddef>;
import <cstdint>;
import <cstdio>;
import <cstdlib>;
import <filesystem>;
import <functional>;
import <future>;
import <initializer_list>;
import <list>;
import <map>;
import <memory_resource>;
import <mutex>;
import <numbers>;
import <numeric>;
import <optional>;
import <random>;
import <ranges>;
import <set>;
import <shared_mutex>;
import <source_location>;
import <span>;
import <sstream>;
import <stdexcept>;
import <string>;
import <thread>;
import <tuple>;
import <typeinfo>;
import <typeindex>;
import <type_traits>;
import <vector>;
import <unordered_set>;

// 3rd party header units. most 3rd party headers are not importable, but some are.
// clang-format off
// clang-format on

// first party is always true modules, no header units allowed.