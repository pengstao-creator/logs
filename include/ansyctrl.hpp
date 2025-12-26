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
            AnsyCtrlCommon();
            ~AnsyCtrlCommon() override { stop(); }
            void stop() override;
            void push(const std::string &str) override;
            void bindcallbackf(const CallbackF &cf);

        private:
            void HandleBuffer() override;

        private:
            std::thread _th;
            std::condition_variable _por;
            std::condition_variable _con;
        };

        class AnsyCtrlThpool : public AnsyCtrl, public std::enable_shared_from_this<AnsyCtrlThpool>
        {
        public:
            AnsyCtrlThpool();
            ~AnsyCtrlThpool() override { stop(); }
            void stop() override;
            void push(const std::string &str) override;
            void bindcallbackf(const CallbackF &cf);

        private:
            void HandleBuffer() override;
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
