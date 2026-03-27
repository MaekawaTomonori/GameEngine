#include "Scheduler.hpp"

#include <utility>

Scheduler::Scheduler() {
    workers_ = std::thread(&Scheduler::Work, this);
}

Scheduler::~Scheduler() {
    Shutdown();
}

void Scheduler::RunTaskLater(const Task& _task, std::chrono::milliseconds _delay) {

    {
        const auto& executionTime = std::chrono::system_clock::now() + _delay;
        std::scoped_lock lock(mutex_);
        taskQueue_.push({_task, executionTime});
    }
    condition_.notify_one();
}

void Scheduler::RunTaskTimer(const Task& _task, std::chrono::milliseconds _interval) {
    (void)_task;
    (void)_interval;
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
