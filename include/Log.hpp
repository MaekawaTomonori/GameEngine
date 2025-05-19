#ifndef LOG_HPP
#define LOG_HPP

#include <string>

#include <lwlog.h>

class Log {
public:
	enum class Level{
		TRACE = 0,
		DEBUG = 1,
		INFO = 2,
		WARNING = 3,
		ERR = 4,
		FATAL = 5,
	};

private:
	Level level_ = Level::INFO;
	std::string path_ = "Logs/";
	std::unique_ptr<lwlog::logger> logger_;

	std::mutex mutex_;
public:
	void Initialize();

	static void Send(Level level, const std::string& message);
};



#endif //LOG_HPP
