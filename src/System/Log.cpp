#include "include/Log.hpp"

#ifdef _WIN32
#endif

void Log::Initialize() {
	std::lock_guard<std::mutex> lock(mutex_);

	if (std::filesystem::exists(path_)) {
		try {
			std::filesystem::create_directories(path_);
		} catch (...) {
			return;
		}
	}

	lwlog::set_pattern("[%T] [%L] : %v");

}

void Log::Send(Level _level = Level::INFO, const std::string& message = "") {
	(void)_level;
	(void)message;
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