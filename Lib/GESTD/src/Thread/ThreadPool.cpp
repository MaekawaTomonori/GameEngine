#include "Thread/ThreadPool.hpp"

#include <algorithm>

namespace GESTD {

    ThreadPool::~ThreadPool() {
        Stop();
    }

    void ThreadPool::Start(const uint32_t _threadCount) {
        // 起動したワーカーはWorkerLoopでmutex_を待つため、Start完了までブロックされる
        std::lock_guard<std::mutex> lock(mutex_);

        if (!workers_.empty()) return;
        stop_ = false;

        uint32_t count = _threadCount;
        if (count == 0) {
            const uint32_t hardware = std::thread::hardware_concurrency();
            count = std::max(1u, hardware / 2);
        }

        workers_.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            workers_.emplace_back([this]() { WorkerLoop(); });
        }
    }

    void ThreadPool::Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (workers_.empty()) return;
            stop_ = true;
        }

        condition_.notify_all();

        for (std::thread& worker : workers_) {
            if (worker.joinable()) worker.join();
        }

        std::lock_guard<std::mutex> lock(mutex_);
        workers_.clear();
        std::queue<Task>().swap(tasks_);
    }

    void ThreadPool::Enqueue(Task _task) {
        if (!_task) return;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!workers_.empty() && !stop_) {
                tasks_.push(std::move(_task));
                condition_.notify_one();
                return;
            }
        }

        // 未起動なら呼び出しスレッドで実行する
        _task();
    }

    bool ThreadPool::IsRunning() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !workers_.empty() && !stop_;
    }

    uint32_t ThreadPool::GetThreadCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return static_cast<uint32_t>(workers_.size());
    }

    void ThreadPool::WorkerLoop() {
        while (true) {
            Task task;

            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

                if (stop_) return;

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            task();
        }
    }

} // namespace GESTD
