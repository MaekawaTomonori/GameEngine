#include "include/Log.hpp"

#include "include/Utils.hpp"
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
    console->set_pattern("%^[%T] [%l] [Engine] [" + workingDirectory_ + "] %v%$");
    sinks_.emplace_back(console);

    // Daily rotating file sink with full execution context
    auto file = std::make_shared<spdlog::sinks::daily_file_sink_mt>(logFilePath_ + logFileName_ + logFileExt_, 0, 0);
    file->set_pattern("[%Y-%m-%d %T] [%l] [Engine] [EXE:" + executablePath_ + "] [CWD:" + workingDirectory_ + "] %v");
    sinks_.emplace_back(file);

#ifdef _WIN32
    // Visual Studio output window sink with execution context
    auto windows_sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    windows_sink->set_pattern("[%T] [%l] [Engine] [" + workingDirectory_ + "] %v");
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

void Log::InitializeExecutionContext() {
    try {
        workingDirectory_ = GetCurrentWorkingDirectory();
        executablePath_ = GetExecutablePath();
    } catch (const std::exception& e) {
        workingDirectory_ = "Error: " + std::string(e.what());
        executablePath_ = "Error: " + std::string(e.what());
    }
}

std::string Log::GetCurrentWorkingDirectory() {
    try {
        return std::filesystem::current_path().string();
    } catch (const std::exception& e) {
        return "Error getting CWD: " + std::string(e.what());
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
    } catch (const std::exception& e) {
        return "Error: " + std::string(e.what());
    }
}

void Log::SendWithContext(Level level, const std::string& message, const std::string& context) {
    std::string contextPrefix = context.empty() ? "" : "[" + context + "] ";
    std::string enhancedMessage = contextPrefix + message;
    Send(level, enhancedMessage);
}

void Log::LogExecutionContext() {
    Send(Level::INFO, "=== Execution Context Information ===");
    Send(Level::INFO, std::format("Executable: {}", executablePath_));
    Send(Level::INFO, std::format("Working Directory: {}", workingDirectory_));
    Send(Level::INFO, "=== End Context Information ===");
}

void Log::LogFileOperation(const std::string& operation, const std::string& filePath, bool success, const std::string& details) {
    std::string status = success ? "SUCCESS" : "FAILED";
    std::string message = std::format("FILE_OP [{}] {} -> {}", status, operation, filePath);
    
    if (!details.empty()) {
        message += " (" + details + ")";
    }
    
    // Check if file exists and add absolute path info
    try {
        if (std::filesystem::exists(filePath)) {
            std::string absolutePath = std::filesystem::absolute(filePath).string();
            message += std::format(" [ABS: {}]", absolutePath);
        } else {
            message += " [FILE_NOT_FOUND]";
        }
    } catch (const std::exception& e) {
        message += std::format(" [PATH_ERROR: {}]", e.what());
    }
    
    Level logLevel = success ? Level::INFO : Level::ERR;
    Send(logLevel, message);
}

// Legacy compatibility functions
void Log::SendWithPath(Level level, const std::string& message, const std::string& context) {
    SendWithContext(level, message, context);
}

void Log::LogWorkingDirectory() {
    LogExecutionContext();
}

void Log::LogFileSystemDiagnostics(const std::string& targetPath, const std::string& context) {
    std::string prefix = context.empty() ? "" : "[" + context + "] ";
    LogFileOperation("DIAGNOSTIC", targetPath, std::filesystem::exists(targetPath), prefix + "File system check");
}


void Log::SetLevel(Level level) {
    level_ = level;
    if (spdlog::get("Engine")) {
        spdlog::get("Engine")->set_level(static_cast<spdlog::level::level_enum>(static_cast<int>(level)));
    }
}