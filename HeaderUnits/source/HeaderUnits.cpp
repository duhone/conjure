// std header units, get rid of these and use import std in actual code files instead, once on C++23. except a few that
// will need to stay here, ones that export macros. like cassert.
import<algorithm>;
import<cassert>;
import<chrono>;
import<compare>;
import<concepts>;
import<cstddef>;
import<cstdint>;
import<cstdio>;
import<filesystem>;
import<functional>;
import<future>;
import<initializer_list>;
import<list>;
import<map>;
import<mutex>;
import<numeric>;
import<optional>;
import<random>;
import<ranges>;
import<set>;
import<shared_mutex>;
import<span>;
import<sstream>;
import<string>;
import<tuple>;
import<type_traits>;
import<vector>;
import<unordered_set>;

// 3rd party header units. most 3rd party headers are not importable, but some are.
// clang-format off
// clang-format on

// first party is always true modules, no header units allowed.