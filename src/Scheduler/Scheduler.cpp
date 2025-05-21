#include "Scheduler.hpp"

#include <utility>

Scheduler::Scheduler() {
	workers_ = std::thread(&Scheduler::Work, this);
}

Scheduler::~Scheduler() {
	Shutdown();
}

void Scheduler::RunTaskLater(Task task, std::chrono::milliseconds delay) {

	{
		const auto& executionTime = std::chrono::system_clock::now() + delay;
		std::lock_guard<std::mutex> lock(mutex_);
		taskQueue_.push({std::move(task), executionTime});
	}
	condition_.notify_one();
}

void Scheduler::RunTaskTimer(Task task, std::chrono::milliseconds interval) {
	(void)task;
	(void)interval;
}

void Scheduler::Work() {
	while (running_){
		std::unique_lock<std::mutex> lock(mutex_);
		condition_.wait(lock, [this]{
			return !taskQueue_.empty() || !running_;
		});
		if (!running_) break;
		auto now = std::chrono::system_clock::now();
		while (!taskQueue_.empty() && taskQueue_.top().executionTime <= now){
			auto task = taskQueue_.top().task;
			taskQueue_.pop();
			lock.unlock();
			task();
			lock.lock();
		}
	}
}

void Scheduler::Shutdown() {
	{
		std::lock_guard<std::mutex> lock(mutex_);

		if (running_){
			running_ = false;
			condition_.notify_all();
		}
	}

	if (workers_.joinable()){
		workers_.join();
	}
}
