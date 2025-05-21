#include "include/Log.hpp"

#include "include/Utils.hpp"
#include "src/DirectX/DirectXAdaptor.hpp"

#ifdef _WIN32
#endif

void Log::Initialize() {
	/*auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console->set_pattern("%^[%T] %v%$");
	sinks_.emplace_back(console);

	auto file = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(logFilePath_ + logFileName_ + logFileExt_, 0, 0);
	file->set_pattern("%^[%T] %v%$");
	sinks_.emplace_back(file);

	auto logger = std::make_shared<spdlog::logger>("Logger", begin(sinks_), end(sinks_));
	logger->set_level(static_cast<spdlog::level::level_enum>(level_));

#ifdef _WIN32
	auto windows_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
	sinks_.emplace_back(windows_sink);
#endif

	spdlog::register_logger(logger);

	spdlog::flush_every(std::chrono::seconds(1));
	spdlog::init_thread_pool(8192, 2);
	spdlog::set_default_logger(
		std::make_shared<spdlog::async_logger>(
			"async",
			begin(sinks_),
			end(sinks_),
			spdlog::thread_pool()
		)
	);

	spdlog::info("Log initialized");*/
}

void Log::Send(Level _level = Level::INFO, const std::string& message = "") {
	(void)_level;
	std::string format = "[" + Utils::DateToString() + "] ";
	OutputDebugStringA((format + message + '\n').c_str());

	//// Implement logging logic here
	//// For example, write to a file or console based on the log level
	//switch (_level) {
	//	case Level::TRACE:
	//		// Log trace message
	//		break;
	//	case Level::DEBUG:
	//		// Log debug message
	//		break;
	//	case Level::INFO:
	//		// Log info message
	//		spdlog::info(message.c_str());
	//		break;
	//	case Level::WARNING:
	//		// Log warning message
	//		break;
	//	case Level::ERROR:
	//		// Log error message
	//		break;
	//	case Level::FATAL:
	//		// Log fatal message
	//		break;
	//}
}