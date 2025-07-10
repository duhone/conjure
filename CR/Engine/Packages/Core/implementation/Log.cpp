module;

#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

module CR.Engine.Core.Log;

import std;

namespace {
	std::shared_ptr<spdlog::async_logger>& getAsyncLogger() {
		static std::shared_ptr<spdlog::async_logger> s_logger;
		return s_logger;
	}
}    // namespace

CR::Engine::Core::LogSystem::LogSystem() {
	assert(!getAsyncLogger().get());

	spdlog::init_thread_pool(8192, 1);
	auto stdSink  = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
	std::vector<spdlog::sink_ptr> sinks{stdSink, msvcSink};

	getAsyncLogger() =
	    std::make_shared<spdlog::async_logger>("default_logger", sinks.begin(), sinks.end(),
	                                           spdlog::thread_pool(), spdlog::async_overflow_policy::block);

	getAsyncLogger()->set_pattern("[%H:%M:%S:%f] [thread %t] [%^%l%$] %v ");
	getAsyncLogger()->flush_on(spdlog::level::warn);
	spdlog::register_logger(getAsyncLogger());
}

CR::Engine::Core::LogSystem::~LogSystem() {
	assert(getAsyncLogger().get());

	getAsyncLogger()->flush();
	getAsyncLogger().reset();
	spdlog::drop_all();
	spdlog::shutdown();
}

spdlog::logger* CR::Engine::Core::Log::GetLogger() {
	assert(getAsyncLogger().get());
	return getAsyncLogger().get();
}