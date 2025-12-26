#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>
#include <memory>
#include <chrono>
#include <iostream>
#include "logdata.hpp"

class threadPool
{
public:
    explicit threadPool(size_t threadCount = std::thread::hardware_concurrency());
    ~threadPool();

    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();

        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            if (_stop)
                throw std::runtime_error("enqueue on _stopped _threadPool");

            tasks.emplace([task]()
                          { (*task)(); });
        }

        _condition.notify_one();
        return res;
    }

    template <class F, class... Args>
    void addLogTask(F &&f, Args &&...args)
    {
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            if (_stop)
                return;

            tasks.emplace(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        }
        _condition.notify_one();
    }

    size_t pendingTasks() const;
    size_t activeThreads() const;
    void stop();

private:
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> tasks;

    mutable std::mutex _queueMutex;
    std::condition_variable _condition;
    std::atomic<bool> _stop;
};

class GlobalTPool
{
public:
    static GlobalTPool &getInstance();
    void initialize(size_t threadCount = Log::Data::threadCount());
    threadPool &get_threadPool();
    
    template <class F, class... Args>
    void enqueue(F &&f, Args &&...args)
    {
        std::lock_guard<std::mutex> lock(_initMutex);
        if (!_threadPool)
        {
            auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
            task();
            return;
        }
        _threadPool->addLogTask(std::forward<F>(f), std::forward<Args>(args)...);
    }

    size_t getPendingTaskCount();
    size_t getActiveThreadCount();
    void shutdown();

    GlobalTPool(const GlobalTPool &) = delete;
    GlobalTPool &operator=(const GlobalTPool &) = delete;
    GlobalTPool(GlobalTPool &&) = delete;
    GlobalTPool &operator=(GlobalTPool &&) = delete;

private:
    GlobalTPool() = default;
    ~GlobalTPool()
    {
        shutdown();
    }

    std::shared_ptr<threadPool> _threadPool;
    std::mutex _initMutex;
    bool initialized = false;
};

#define LOG_THREAD_POOL GlobalTPool::getInstance()
