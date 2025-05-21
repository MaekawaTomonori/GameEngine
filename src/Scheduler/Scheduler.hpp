#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include <chrono>
#include <functional>
#include <mutex>
#include <queue>

class Scheduler {
public:
	using Task = std::function<void()>;

	Scheduler();
	~Scheduler();

	void RunTaskLater(Task task, std::chrono::milliseconds delay);
	void RunTaskTimer(Task task, std::chrono::milliseconds interval);

private:
	struct SchedulerTask {
		Task task;
		std::chrono::system_clock::time_point executionTime;

		bool operator<(const SchedulerTask& other) const {
			return executionTime < other.executionTime; // For priority queue
		}
		bool operator>(const SchedulerTask& other) const {
			return executionTime > other.executionTime; // For priority queue
		}
	};

	void Work();
	void Shutdown();

	std::priority_queue<SchedulerTask, std::vector<SchedulerTask>, std::greater<>> taskQueue_;
	std::mutex mutex_;
	std::condition_variable condition_;

	std::atomic<bool> running_;

	std::thread workers_;
};



#endif //SCHEDULER_HPP
