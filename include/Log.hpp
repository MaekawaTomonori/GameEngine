#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

// Forward declarations
namespace spdlog {
    class logger;
    namespace sinks {
        class sink;
    }
}

class Log {
public:
    enum class Level{
        TRACE = 0,
        DBG = 1,
        INFO = 2,
        WARNING = 3,
        ERR = 4,
        FATAL = 5,
    };

private:
    static std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks_;
    static Level level_;
    static std::string logFilePath_;
    static std::string logFileName_;
    static std::string logFileExt_;
    static std::string executablePath_;
    static std::string workingDirectory_;

public:
    static void Initialize();
    static void Send(Level level, const std::string& message);
    static void SendWithContext(Level level, const std::string& message, const std::string& context = "");
    static void LogExecutionContext();
    static void LogFileOperation(const std::string& operation, const std::string& filePath, bool success = true, const std::string& details = "");
    static void SetLevel(Level level);
    
private:
    static void InitializeExecutionContext();
    static std::string GetCurrentWorkingDirectory();
    static std::string GetExecutablePath();
    
    // Legacy compatibility - will be removed
    static void LogWorkingDirectory();
    static void LogFileSystemDiagnostics(const std::string& targetPath, const std::string& context = "");
    static void SendWithPath(Level level, const std::string& message, const std::string& context = "");
};

#endif //LOG_HPP
