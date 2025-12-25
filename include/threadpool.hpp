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
    explicit threadPool(size_t threadCount = std::thread::hardware_concurrency())
        : _stop(false)
    {
        for (size_t i = 0; i < threadCount; ++i)
        {
            _workers.emplace_back([this]
                                 {
                for (;;) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(this->_queueMutex);
                        this->_condition.wait(lock, [this] {
                            return this->_stop || !this->tasks.empty();
                        });
                        
                        if (this->_stop && this->tasks.empty())
                            return;
                            
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                        task();          
                } });
        }
    }

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

    // 专门为日志优化的添加任务方法（不需要返回结果）
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

    size_t pendingTasks() const
    {
        std::unique_lock<std::mutex> lock(_queueMutex);
        return tasks.size();
    }

    size_t activeThreads() const
    {
        return _workers.size();
    }

    ~threadPool()
    {
        if (!_stop)
            stop();
    }

    void stop()
    {
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _stop = true;
        }

        _condition.notify_all();

        for (std::thread &worker : _workers)
        {
            if (worker.joinable())
                worker.join();
        }
    }

private:
    std::vector<std::thread> _workers;
    std::queue<std::function<void()>> tasks;

    mutable std::mutex _queueMutex;
    std::condition_variable _condition;
    std::atomic<bool> _stop;
};
// GlobalTPool.h

class GlobalTPool
{
public:
    // 获取单例实例
    static GlobalTPool &getInstance()
    {
        static GlobalTPool instance;
        return instance;
    }

    // 初始化线程池
    void initialize(size_t threadCount = Log::Data::threadCount())
    {
        if (_threadPool)
            return;
        std::lock_guard<std::mutex> lock(_initMutex);
        if (!_threadPool)
        {
            _threadPool = std::make_shared<threadPool>(threadCount);
            
            initialized = true;
        }
    }

    // 获取线程池引用
    threadPool &get_threadPool()
    {
        if (!initialized)
        {
            initialize(); // 默认初始化
        }
        return *_threadPool;
    }

    // 添加日志任务
    template <class F, class... Args>
    void enqueue(F &&f, Args &&...args)
    {
        std::lock_guard<std::mutex> lock(_initMutex);
        if (!_threadPool)
        {
            // 如果线程池不存在，直接执行任务避免丢失数据
        // 使用bind和直接调用代替std::invoke（C++14兼容）
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        task(); // 直接执行任务，无需条件判断
        return;
        }
        _threadPool->addLogTask(std::forward<F>(f), std::forward<Args>(args)...);
    }

    // 获取待处理任务数
    size_t getPendingTaskCount()
    {
        if (!initialized)
            return 0;
        return _threadPool->pendingTasks();
    }

    // 获取活动线程数
    size_t getActiveThreadCount()
    {
        if (!initialized)
            return 0;
        return _threadPool->activeThreads();
    }

    // 安全关闭（等待所有任务完成，带超时机制）
    void shutdown()
    {
        if (_threadPool)
        {
            // 先锁定初始化互斥锁，确保没有新任务被加入
            std::lock_guard<std::mutex> lock(_initMutex);
            
            // 等待队列中的任务完成，最多等待5秒
            auto start = std::chrono::high_resolution_clock::now();
            auto timeout = std::chrono::seconds(5);
            
            while (getPendingTaskCount() > 0)
            {
                auto now = std::chrono::high_resolution_clock::now();
                if (now - start > timeout)
                {
                    break; // 超时，直接关闭
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            // 停止线程池并重置
            _threadPool->stop();
            _threadPool.reset();
            initialized = false;
        }
    }

    // 禁止拷贝和移动
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

// 全局访问宏（可选）
#define LOG_THREAD_POOL GlobalTPool::getInstance()