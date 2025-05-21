#ifndef Timer_HPP_
#define Timer_HPP_
#include <chrono>

class Timer {
	std::chrono::system_clock::time_point start_;
	std::chrono::milliseconds duration_;

	bool isStarted_ = false;

public:
	Timer(std::chrono::milliseconds _duration);
	void Start();
	void Stop();
	void Reset();
	void Restart();

	bool Check();

	void SetDuration(std::chrono::milliseconds _duration);
}; // class Timer

#endif // Timer_HPP_
