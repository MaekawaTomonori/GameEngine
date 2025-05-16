//
// Created by tomo- on 25/05/07.
//

#ifndef LOG_HPP
#define LOG_HPP

#include <string>

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
	std::string logFilePath_ = "Log/";
	std::string logFileName_ = "LatestLog";
	std::string logFileExt_ = ".log";
public:
	void Initialize();

	static void Send(Level level, const std::string& message);
};



#endif //LOG_HPP
