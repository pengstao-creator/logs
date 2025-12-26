#include "../include/pch.hpp"
#include "../include/threadpool.hpp"

GlobalTPool &GlobalTPool::getInstance()
{
    static GlobalTPool instance;
    return instance;
}

threadPool::threadPool(size_t threadCount)
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

threadPool::~threadPool()
{
    if (!_stop)
        stop();
}

void threadPool::stop()
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

size_t threadPool::pendingTasks() const
{
    std::unique_lock<std::mutex> lock(_queueMutex);
    return tasks.size();
}

size_t threadPool::activeThreads() const
{
    return _workers.size();
}

void GlobalTPool::initialize(size_t threadCount)
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

threadPool &GlobalTPool::get_threadPool()
{
    if (!initialized)
    {
        initialize();
    }
    return *_threadPool;
}

size_t GlobalTPool::getPendingTaskCount()
{
    if (!initialized)
        return 0;
    return _threadPool->pendingTasks();
}

size_t GlobalTPool::getActiveThreadCount()
{
    if (!initialized)
        return 0;
    return _threadPool->activeThreads();
}

void GlobalTPool::shutdown()
{
    if (_threadPool)
    {
        std::lock_guard<std::mutex> lock(_initMutex);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto timeout = std::chrono::seconds(5);
        
        while (getPendingTaskCount() > 0)
        {
            auto now = std::chrono::high_resolution_clock::now();
            if (now - start > timeout)
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        _threadPool->stop();
        _threadPool.reset();
        initialized = false;
    }
}
