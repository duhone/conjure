module;

#include <fmt/format.h>
#include <spdlog/async_logger.h>
#include <spdlog/spdlog.h>

export module CR.Engine.Core.Log;

import<cassert>;
import<memory>;
import<thread>;

namespace CR::Engine::Core::Log {
	class Logger final {
	  public:
		Logger();
		~Logger();
		Logger(const Logger&) = delete;
		Logger(Logger&&)      = delete;
		Logger& operator=(const Logger&) = delete;
		Logger& operator=(Logger&&) = delete;

		template<typename... ArgTs>
		void Debug(const char* a_fmt, ArgTs&&... a_args) {
			m_logger->debug(a_fmt, std::forward<ArgTs>(a_args)...);
		}

		template<typename... ArgTs>
		void Info(const char* a_fmt, ArgTs&&... a_args) {
			m_logger->info(a_fmt, std::forward<ArgTs>(a_args)...);
		}

		template<typename... ArgTs>
		void Warn(const char* a_fmt, ArgTs&&... a_args) {
			m_logger->warn(a_fmt, std::forward<ArgTs>(a_args)...);
		}

		template<typename... ArgTs>
		void Error(const char* a_fmt, ArgTs&&... a_args) {
			m_logger->error(a_fmt, std::forward<ArgTs>(a_args)...);
			m_logger->flush();
			throw std::exception(fmt::format(a_fmt, std::forward<ArgTs>(a_args)...).c_str());
		}

	  private:
		void Free();

		std::shared_ptr<spdlog::async_logger> m_logger;
	};

	Logger& GetLogger();
}    // namespace CR::Engine::Core::Log

export namespace CR::Engine::Core::Log {
	template<typename... ArgTs>
	inline void Debug([[maybe_unused]] const char* a_fmt, [[maybe_unused]] ArgTs&&... a_args) {
		if constexpr(CR_DEBUG) { GetLogger().Debug(a_fmt, std::forward<ArgTs>(a_args)...); }
	}

	template<typename... ArgTs>
	inline void Info([[maybe_unused]] const char* a_fmt, [[maybe_unused]] ArgTs&&... a_args) {
		if constexpr(CR_DEBUG || CR_RELEASE) { GetLogger().Info(a_fmt, std::forward<ArgTs>(a_args)...); }
	}

	template<typename... ArgTs>
	inline void Warn(const char* a_fmt, ArgTs&&... a_args) {
		if constexpr(CR_DEBUG || CR_RELEASE) {
			GetLogger().Error(a_fmt, std::forward<ArgTs>(a_args)...);
		} else {
			GetLogger().Warn(a_fmt, std::forward<ArgTs>(a_args)...);
		}
	}

	template<typename... ArgTs>
	inline void Error(const char* a_fmt, ArgTs&&... a_args) {
		GetLogger().Error(a_fmt, std::forward<ArgTs>(a_args)...);
	}

	template<typename... ArgTs>
	inline void Assert(bool condition, [[maybe_unused]] const char* a_fmt, [[maybe_unused]] ArgTs&&... a_args) {
		if constexpr(CR_DEBUG || CR_RELEASE) {
			if(!condition) { GetLogger().Error(a_fmt, std::forward<ArgTs>(a_args)...); }
		} else {
			__assume(!condition);
		}
	}

	template<typename... ArgTs>
	inline void Require(bool condition, const char* a_fmt, ArgTs&&... a_args) {
		if(!condition) { GetLogger().Error(a_fmt, std::forward<ArgTs>(a_args)...); }
	}
}    // namespace CR::Engine::Core::Log
