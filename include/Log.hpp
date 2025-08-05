#ifndef LOG_HPP
#define LOG_HPP

#include <string>
#include <vector>
#include <memory>

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

public:
    static void Initialize();
    static void Send(Level level, const std::string& message);
    static void SetLevel(Level level);
};

#endif //LOG_HPP
