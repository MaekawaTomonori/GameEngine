#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace GESTD {

    /** @brief 固定数のワーカーでタスクを並列実行するプール
     *
     * 用途ごとにインスタンスを分けて持つことを想定している。
     * フレーム内に完了すべき処理と長時間ジョブを同じプールに混ぜると、
     * 後者が前者を待たせるため共有しない。
     */
    class ThreadPool {
    public:
        using Task = std::function<void()>;

        ThreadPool() = default;
        ~ThreadPool();

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        /** @brief ワーカーを起動する
         * @param _threadCount 起動数。0なら hardware_concurrency の半分（最低1）
         */
        void Start(uint32_t _threadCount = 0);

        /** @brief ワーカーを停止する
         * 実行中のタスクの完了は待つが、キューに残ったタスクは破棄する
         */
        void Stop();

        /** @brief タスクを投入する
         * 未起動の場合は呼び出しスレッドでその場で実行する
         */
        void Enqueue(Task _task);

        bool IsRunning() const;
        uint32_t GetThreadCount() const;

    private:
        void WorkerLoop();

        std::vector<std::thread> workers_;
        std::queue<Task> tasks_;

        mutable std::mutex mutex_;
        std::condition_variable condition_;

        bool stop_ = false;
    }; // class ThreadPool

} // namespace GESTD

#endif // THREADPOOL_HPP_
