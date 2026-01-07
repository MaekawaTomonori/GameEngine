#include "include/Log.hpp"

#include "src/DirectX/DirectXAdapter.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>
#include <chrono>
#include <filesystem>
#include <format>

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
std::string Log::executablePath_;
std::string Log::workingDirectory_;

void Log::Initialize() {
    // Initialize execution context information
    InitializeExecutionContext();

    // Clear any existing sinks
    sinks_.clear();

    // Console sink with execution context in pattern
    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console->set_pattern("%^[%T] [%l] %v%$");
    sinks_.emplace_back(console);

    // Daily rotating file sink with full execution context
    auto file = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logFilePath_ + logFileName_ + logFileExt_, 0, 0);
    file->set_pattern("[%Y-%m-%d %T] [%l] %v");
    sinks_.emplace_back(file);

#ifdef _WIN32
    // Visual Studio output window sink with execution context
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

    // Log execution context immediately after initialization
    LogExecutionContext();
}

void Log::Send(Level _level, const std::string& _message) {
    switch (_level) {
        case Level::TRACE:
            spdlog::trace(_message);
            break;
        case Level::DBG:
            spdlog::debug(_message);
            break;
        case Level::INFO:
            spdlog::info(_message);
            break;
        case Level::WARNING:
            spdlog::warn(_message);
            break;
        case Level::ERR:
            spdlog::error(_message);
            break;
        case Level::FATAL:
            spdlog::critical(_message);
            break;
    }
}

void Log::Send(const std::string& _message) {
    spdlog::debug(_message);
}

void Log::InitializeExecutionContext() {
    try {
        workingDirectory_ = GetCurrentWorkingDirectory();
        executablePath_ = GetExecutablePath();
    } catch (const std::exception& _e) {
        workingDirectory_ = "Error: " + std::string(_e.what());
        executablePath_ = "Error: " + std::string(_e.what());
    }
}

std::string Log::GetCurrentWorkingDirectory() {
    try {
        return std::filesystem::current_path().string();
    } catch (const std::exception& _e) {
        return "Error getting CWD: " + std::string(_e.what());
    }
}

std::string Log::GetExecutablePath() {
    try {
#ifdef _WIN32
        char exePath[MAX_PATH];
        DWORD result = GetModuleFileNameA(NULL, exePath, MAX_PATH);
        if (result > 0) {
            return std::string(exePath);
        }
        return "Error getting executable path";
#else
        return "Non-Windows platform";
#endif
    } catch (const std::exception& _e) {
        return "Error: " + std::string(_e.what());
    }
}

void Log::SendWithContext(Level _level, const std::string& _message, const std::string& _context) {
    std::string contextPrefix = _context.empty() ? "" : "[" + _context + "] ";
    std::string enhancedMessage = contextPrefix + _message;
    Send(_level, enhancedMessage);
}

void Log::LogExecutionContext() {
    Send(Level::INFO, "=== Execution Context Information ===");
    Send(Level::INFO, std::format("Executable: {}", executablePath_));
    Send(Level::INFO, std::format("Working Directory: {}", workingDirectory_));
    Send(Level::INFO, "=== End Context Information ===");
}

void Log::LogFileOperation(const std::string& _operation, const std::string& _filePath, bool _success, const std::string& _details) {
    std::string status = _success ? "SUCCESS" : "FAILED";
    std::string message = std::format("FILE_OP [{}] {} -> {}", status, _operation, _filePath);

    if (!_details.empty()) {
        message += " (" + _details + ")";
    }

    // Check if file exists and add absolute path info
    try {
        if (std::filesystem::exists(_filePath)) {
            std::string absolutePath = std::filesystem::absolute(_filePath).string();
            message += std::format(" [ABS: {}]", absolutePath);
        } else {
            message += " [FILE_NOT_FOUND]";
        }
    } catch (const std::exception& _e) {
        message += std::format(" [PATH_ERROR: {}]", _e.what());
    }

    Level logLevel = _success ? Level::INFO : Level::ERR;
    Send(logLevel, message);
}

// Legacy compatibility functions
void Log::SendWithPath(Level _level, const std::string& _message, const std::string& _context) {
    SendWithContext(_level, _message, _context);
}

void Log::LogWorkingDirectory() {
    LogExecutionContext();
}

void Log::LogFileSystemDiagnostics(const std::string& _targetPath, const std::string& _context) {
    std::string prefix = _context.empty() ? "" : "[" + _context + "] ";
    LogFileOperation("DIAGNOSTIC", _targetPath, std::filesystem::exists(_targetPath), prefix + "File system check");
}


void Log::SetLevel(Level _level) {
    level_ = _level;
    if (spdlog::get("Engine")) {
        spdlog::get("Engine")->set_level(static_cast<spdlog::level::level_enum>(static_cast<int>(_level)));
    }
}
