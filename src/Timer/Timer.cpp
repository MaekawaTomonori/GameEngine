#include "Timer.hpp"

#include "include/Utils.hpp"

Timer::Timer(std::chrono::milliseconds _duration) :duration_(_duration), isStarted_(false){
}

void Timer::Start() {
	if (isStarted_){
		Utils::Alert("Timer already started");
		return; // Already started
	}
	start_ = std::chrono::system_clock::now();
	isStarted_ = true;
}

void Timer::Stop() {
	if (isStarted_){
		auto now = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_);
		if (elapsed >= duration_){
			isStarted_ = false;
			return;
		}

		Reset();
	}
}

void Timer::Reset() {
	isStarted_ = false;
}

void Timer::Restart() {
	Reset();
	Start();
}

bool Timer::Check() {
	if (!isStarted_){
		Utils::Alert("Timer not started");
		return false; // Not started
	}
	auto now = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_);
	if (elapsed >= duration_){
		isStarted_ = false;
		return true; // Timer expired
	}
	return false; // Timer not expired
}

void Timer::SetDuration(std::chrono::milliseconds _duration) {
	duration_ = _duration;
}
