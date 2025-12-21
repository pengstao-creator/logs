#pragma once
#include "buffer.hpp"
#include "threadpool.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

/*
    异步线程控制器
    创建线程进行缓冲区分离，控制日志写入缓冲区
    通过生产消费模型进行线程控制
    通过回掉函数将缓冲区内容实质落地
*/

namespace Log
{
    namespace ACtrl
    {
        class AnsyCtrl
        {

        public:
            typedef std::function<void(const std::string &Buffer)> CallbackF;
            typedef std::shared_ptr<AnsyCtrl> ptr;
            AnsyCtrl()
                : _stop(false)
            {
            }
            virtual void bindcallbackf(const CallbackF &) = 0;
            virtual void stop() = 0;
            virtual void push(const std::string &str) = 0;
            virtual ~AnsyCtrl() {};

        protected:
            virtual void HandleBuffer() = 0;

        protected:
            std::atomic<bool> _stop;
            CallbackF _callbackf;
            Buffer _por_buf;
            Buffer _con_buf;
            std::mutex _mutex;
        };
        class AnsyCtrlCommon : public AnsyCtrl
        {

        public:
            AnsyCtrlCommon()
                : _th(std::bind(&AnsyCtrlCommon::HandleBuffer, this))
            {
            }
            ~AnsyCtrlCommon() { stop(); }
            void stop() override
            {
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    _stop = true;
                }
                _con.notify_all();
                if (_th.joinable())
                    _th.join();
            }
            void push(const std::string &str) override
            {
                std::unique_lock<std::mutex> lock(_mutex);
                _por.wait(lock, [&]()
                          { return !_stop && str.size() <= _por_buf.WriteBSize(); });
                _por_buf.push(str);
                _con.notify_all();
            }
            void bindcallbackf(const CallbackF &cf)
            {
                _callbackf = cf;
            }

        private:
            void HandleBuffer() override
            {
                while (true)
                {
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        _con.wait(lock, [&]()
                                  { return !_por_buf.empty() || _stop; });
                        if (_stop && _por_buf.empty())
                            break;
                        _con_buf.clear();
                        _por_buf.swap(_con_buf);
                        _por.notify_all();
                    }
                    _callbackf(_con_buf.ReadBuffer());
                }
            }

        private:
            std::thread _th;
            std::condition_variable _por;
            std::condition_variable _con;
        };

        class AnsyCtrlThpool : public AnsyCtrl, public std::enable_shared_from_this<AnsyCtrlThpool>
        {
        public:
            AnsyCtrlThpool()
            {
            }
            ~AnsyCtrlThpool() override
            {
                // 析构函数中不做复杂操作，只标记停止
                std::unique_lock<std::mutex> lock(_mutex);
                _stop = true;
            }
            void stop() override
            {
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    if (_stop)
                        return;
                    _stop = true;
                }

                // 处理剩余的缓冲区内容
                if (!_por_buf.empty())
                {
                    // 使用局部变量保存回调函数和缓冲区内容
                    HandleBuffer();
                }
            }
            void push(const std::string &str) override
            {
                std::unique_lock<std::mutex> lock(_mutex);
                if (_stop || str.size() > _por_buf.WriteBSize())
                    return;

                _por_buf.push(str);

                // 当缓冲区达到一定大小后，使用线程池处理
                // 这里设置一个简单的阈值：当剩余空间小于1024字节时处理
                if (_por_buf.WriteBSize() < 1024)
                {
                    // 创建一个共享指针副本，避免对象被销毁
                    auto self = shared_from_this();
                    lock.unlock();

                    // 使用全局线程池处理缓冲区
                    GlobalTPool::getInstance().enqueue([self]()
                                                       { self->HandleBuffer(); });
                }
            }
            void bindcallbackf(const CallbackF &cf)
            {
                _callbackf = cf;
            }

        private:
            void HandleBuffer() override
            {
                // 这个方法现在不再直接调用，而是通过线程池中的任务来执行
                // 保留这个方法是为了兼容基类接口
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    if (_por_buf.empty() || _stop)
                        return;
                    _con_buf.clear();
                    _por_buf.swap(_con_buf);
                }

                if (_callbackf)
                {
                    _callbackf(_con_buf.ReadBuffer());
                }
            }
        };

    } // neamspace ACtrl

    class ACtrlFactory
    {
    public:
        template <class AnsyWay, class... Args>
        static ACtrl::AnsyCtrl::ptr ACtrlWay(Args &&...args)
        {
            return std::make_shared<AnsyWay>(std::forward<Args>(args)...);
        }

        static ACtrl::AnsyCtrl::ptr AnsyCommon()
        {
            return std::make_shared<ACtrl::AnsyCtrlCommon>();
        }

        static ACtrl::AnsyCtrl::ptr AnsyThpool()
        {
            return std::make_shared<ACtrl::AnsyCtrlThpool>();
        }
    };

} // neamspace Log
