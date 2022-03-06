module;

#include <fmt/format.h>
#include <spdlog/async_logger.h>
#include <spdlog/spdlog.h>

export module CR.Engine.Core.Log;

import<cassert>;
import<memory>;
import<source_location>;
import<thread>;

// max size of user format string plus anything we add to the output.
// Final output can be larger once the users arguments are added in.
constexpr uint32_t c_maxFmtStringSize = 256;

void PreFormat(char* output, const char* fmt, const std::source_location& a_location) {
	// print a condensed path, starting just below CR/
	const char* start = strstr(a_location.file_name(), "CR");
	assert(start != nullptr);
	if(start == nullptr) {
		// couldnt find CR, shouldnt be possible. files should never exists outside this folder.
		start = a_location.file_name();
	} else {
		// skip past CR/
		start += 3;
	}
	auto result = fmt::format_to_n(output, c_maxFmtStringSize - 1, FMT_STRING("[{}:{}] - {}"), start,
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
		LogSystem& operator=(LogSystem&&) = delete;
	};

	namespace Log {
		spdlog::async_logger* GetLogger();

		export template<typename... ArgTs>
		inline void Info(const std::source_location& a_location, const char* a_fmt, ArgTs&&... a_args) {
			char buffer[c_maxFmtStringSize];
			PreFormat(buffer, a_fmt, a_location);
			GetLogger()->info(buffer, std::forward<ArgTs>(a_args)...);
		}

		export template<typename... ArgTs>
		inline void Warn(const std::source_location& a_location, const char* a_fmt, ArgTs&&... a_args) {
			char buffer[c_maxFmtStringSize];
			PreFormat(buffer, a_fmt, a_location);
			GetLogger()->warn(buffer, std::forward<ArgTs>(a_args)...);
		}

		export template<typename... ArgTs>
		inline void Error(const std::source_location& a_location, const char* a_fmt, ArgTs&&... a_args) {
			char buffer[c_maxFmtStringSize];
			PreFormat(buffer, a_fmt, a_location);
			GetLogger()->error(buffer, std::forward<ArgTs>(a_args)...);
			GetLogger()->flush();
			throw std::exception(fmt::format(buffer, std::forward<ArgTs>(a_args)...).c_str());
		}
	}    // namespace Log
}    // namespace CR::Engine::Core
