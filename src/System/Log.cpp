#include "include/Log.hpp"

#include "include/Utils.hpp"
#include "src/DirectX/DirectXAdapter.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>
#include <chrono>

#ifdef _WIN32
#include <spdlog/sinks/msvc_sink.h>
#include <Windows.h>
#endif

// Static member definitions
std::vector<std::shared_ptr<spdlog::sinks::sink>> Log::sinks_;
Log::Level Log::level_ = Level::INFO;
std::string Log::logFilePath_ = "Log/";
std::string Log::logFileName_ = "LatestLog";
std::string Log::logFileExt_ = ".log";

void Log::Initialize() {
    // Clear any existing sinks
    sinks_.clear();

    // Console sink with color support
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console->set_pattern("%^[%T] [%l] %v%$");
    sinks_.emplace_back(console);

    // Daily rotating file sink
    auto file = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logFilePath_ + logFileName_ + logFileExt_, 0, 0);
    file->set_pattern("[%T] [%l] %v");
    sinks_.emplace_back(file);

#ifdef _WIN32
    // Visual Studio output window sink
    auto windows_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    windows_sink->set_pattern("[%T] [%l] %v");
    sinks_.emplace_back(windows_sink);
#endif

    // Create logger with all sinks
    auto logger = std::make_shared<spdlog::logger>("Engine", begin(sinks_), end(sinks_));
    logger->set_level(static_cast<spdlog::level::level_enum>(static_cast<int>(level_)));

    // Register as default logger
    spdlog::set_default_logger(logger);

    // Set up async logging
    spdlog::flush_every(std::chrono::seconds(1));

    spdlog::info("Log system initialized");
}

void Log::Send(Level _level, const std::string& message) {
    switch (_level) {
        case Level::TRACE:
            spdlog::trace(message);
            break;
        case Level::DBG:
            spdlog::debug(message);
            break;
        case Level::INFO:
            spdlog::info(message);
            break;
        case Level::WARNING:
            spdlog::warn(message);
            break;
        case Level::ERR:
            spdlog::error(message);
            break;
        case Level::FATAL:
            spdlog::critical(message);
            break;
    }
}

void Log::SetLevel(Level level) {
    level_ = level;
    if (spdlog::get("Engine")) {
        spdlog::get("Engine")->set_level(static_cast<spdlog::level::level_enum>(static_cast<int>(level)));
    }
}