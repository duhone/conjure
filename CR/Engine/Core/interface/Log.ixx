module;

#include <fmt/compile.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <cassert>

export module CR.Engine.Core.Log;

import <iostream>;
import <memory>;
import <source_location>;
import <thread>;

// max size of user format string plus anything we add to the output.
// Final output can be larger once the users arguments are added in.
constexpr uint32_t c_maxFmtStringSize = 1024;

inline void PreFormat(char* output, const char* fmt, const std::source_location& a_location) {
	// print a condensed path, starting just below CR/
	const char* start = strstr(a_location.file_name(), "CR");
	assert(start != nullptr);
	if(start == nullptr) {
		// couldnt find CR, shouldnt be possible. files should never exists outside this folder.
		assert(false);
		start = a_location.file_name();
	} else {
		// skip past CR/
		start += 3;
	}
	auto result = fmt::format_to_n(output, c_maxFmtStringSize - 1, FMT_COMPILE("[{}:{}] - {}"), start,
	                               a_location.line(), fmt);
	// fmt doesnt append a null terminator for some reason.
	*result.out = '\0';
}

namespace CR::Engine::Core {
	export class LogSystem final {
	  public:
		LogSystem();
		~LogSystem();
		LogSystem(const LogSystem&) = delete;
		LogSystem(LogSystem&&)      = delete;

		LogSystem& operator=(const LogSystem&) = delete;
		LogSystem& operator=(LogSystem&&)      = delete;
	};

	namespace Log {
		spdlog::logger* GetLogger();

		export template<typename... ArgTs>
		inline void Info(const std::source_location& a_location, const char* a_fmt, ArgTs&&... a_args) {
			char buffer[c_maxFmtStringSize];
			PreFormat(buffer, a_fmt, a_location);
			fmt::println(fmt::runtime(buffer), std::forward<ArgTs>(a_args)...);
			GetLogger()->info(fmt::runtime(buffer), std::forward<ArgTs>(a_args)...);
		}

		export template<typename... ArgTs>
		inline void Warn(const std::source_location& a_location, const char* a_fmt, ArgTs&&... a_args) {
			char buffer[c_maxFmtStringSize];
			PreFormat(buffer, a_fmt, a_location);
			fmt::println(fmt::runtime(buffer), std::forward<ArgTs>(a_args)...);
			GetLogger()->warn(fmt::runtime(buffer), std::forward<ArgTs>(a_args)...);
		}

		export template<typename... ArgTs>
		inline void Error(const std::source_location& a_location, const char* a_fmt, ArgTs&&... a_args) {
			char buffer[c_maxFmtStringSize];
			PreFormat(buffer, a_fmt, a_location);
			fmt::println(fmt::runtime(buffer), std::forward<ArgTs>(a_args)...);
			GetLogger()->error(fmt::runtime(buffer), std::forward<ArgTs>(a_args)...);
			GetLogger()->flush();
			__debugbreak();
			std::terminate();
		}
	}    // namespace Log
}    // namespace CR::Engine::Core

module :private;
