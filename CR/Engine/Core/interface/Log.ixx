module;

#include <fmt/format.h>
#include <spdlog/async_logger.h>
#include <spdlog/spdlog.h>

export module CR.Engine.Core.Log;

import<cassert>;
import<memory>;
import<thread>;

export namespace CR::Engine::Core {
	class LogSystem final {
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

		template<typename... ArgTs>
		inline void Info(const char* a_fmt, ArgTs&&... a_args) {
			GetLogger()->info(a_fmt, std::forward<ArgTs>(a_args)...);
		}

		template<typename... ArgTs>
		inline void Warn(const char* a_fmt, ArgTs&&... a_args) {
			GetLogger()->warn(a_fmt, std::forward<ArgTs>(a_args)...);
		}

		template<typename... ArgTs>
		inline void Error(const char* a_fmt, ArgTs&&... a_args) {
			GetLogger()->error(a_fmt, std::forward<ArgTs>(a_args)...);
			GetLogger()->flush();
			throw std::exception(fmt::format(a_fmt, std::forward<ArgTs>(a_args)...).c_str());
		}
	}    // namespace Log
}    // namespace CR::Engine::Core
