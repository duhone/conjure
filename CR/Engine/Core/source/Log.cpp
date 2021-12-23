module;

#include <spdlog/async.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

module CR.Engine.Core.Log;

using namespace CR::Engine::Core;
using namespace std;

Log::Logger::Logger() {
	spdlog::init_thread_pool(8192, 1);
	auto stdSink  = make_shared<spdlog::sinks::stdout_color_sink_mt>();
	auto msvcSink = make_shared<spdlog::sinks::msvc_sink_mt>();
	std::vector<spdlog::sink_ptr> sinks{stdSink, msvcSink};

	m_logger = make_shared<spdlog::async_logger>("default_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(),
	                                             spdlog::async_overflow_policy::block);

	m_logger->set_pattern("[%H:%M:%S:%f] [thread %t] [%^%l%$] %v ");
	m_logger->flush_on(spdlog::level::warn);
	spdlog::register_logger(m_logger);
}

Log::Logger::~Logger() {
	Free();
}

void Log::Logger::Free() {
	if(m_logger) {
		m_logger->flush();
		m_logger.reset();
		spdlog::drop_all();
		spdlog::shutdown();
	}
}

CR::Engine::Core::Log::Logger& CR::Engine::Core::Log::GetLogger() {
	static CR::Engine::Core::Log::Logger logger;
	return logger;
}
