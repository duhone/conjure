module;

#include <spdlog/async.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <cassert>

module CR.Engine.Core.Log;

import<memory>;

std::shared_ptr<spdlog::async_logger> g_logger;

CR::Engine::Core::LogSystem::LogSystem() {
	assert(!g_logger.get());

	spdlog::init_thread_pool(8192, 1);
	auto stdSink  = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
	std::vector<spdlog::sink_ptr> sinks{stdSink, msvcSink};

	g_logger = std::make_shared<spdlog::async_logger>("default_logger", sinks.begin(), sinks.end(),
	                                                  spdlog::thread_pool(), spdlog::async_overflow_policy::block);

	g_logger->set_pattern("[%H:%M:%S:%f] [thread %t] [%^%l%$] %v ");
	g_logger->flush_on(spdlog::level::warn);
	spdlog::register_logger(g_logger);
}

CR::Engine::Core::LogSystem::~LogSystem() {
	assert(g_logger.get());

	g_logger->flush();
	g_logger.reset();
	spdlog::drop_all();
	spdlog::shutdown();
}

/*spdlog::async_logger* CR::Engine::Core::Log::GetLogger() {
	assert(g_logger.get());
	return g_logger.get();
}*/