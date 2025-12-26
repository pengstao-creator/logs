#include "../include/pch.hpp"
#include "../include/ansyctrl.hpp"

namespace Log
{
    ACtrl::AnsyCtrlCommon::AnsyCtrlCommon()
    {
        _th = std::thread([this]() {
            HandleBuffer();
        });
    }

    void ACtrl::AnsyCtrlCommon::stop()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _stop = true;
        }
        _con.notify_all();
        if (_th.joinable())
            _th.join();
    }

    void ACtrl::AnsyCtrlCommon::push(const std::string &str)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _por.wait(lock, [&]()
                  { return !_stop && str.size() <= _por_buf.WriteBSize(); });
        _por_buf.push(str);
        _con.notify_all();
    }

    void ACtrl::AnsyCtrlCommon::bindcallbackf(const CallbackF &cf)
    {
        _callbackf = cf;
    }

    void ACtrl::AnsyCtrlCommon::HandleBuffer()
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
            if (_callbackf)
            {
                _callbackf(_con_buf.ReadBuffer());
            }
        }
    }

    ACtrl::AnsyCtrlThpool::AnsyCtrlThpool()
    {
    }

    void ACtrl::AnsyCtrlThpool::stop()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_stop)
                return;
            _stop = true;
        }

        if (!_por_buf.empty())
        {
            HandleBuffer();
        }
    }

    void ACtrl::AnsyCtrlThpool::push(const std::string &str)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_stop || str.size() > _por_buf.WriteBSize())
            return;

        _por_buf.push(str);

        if (_por_buf.WriteBSize() < 1024)
        {
            auto self = shared_from_this();
            lock.unlock();

            GlobalTPool::getInstance().enqueue([self]()
                                               { self->HandleBuffer(); });
        }
    }

    void ACtrl::AnsyCtrlThpool::bindcallbackf(const CallbackF &cf)
    {
        _callbackf = cf;
    }

    void ACtrl::AnsyCtrlThpool::HandleBuffer()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_por_buf.empty())
                return;
            _con_buf.clear();
            _por_buf.swap(_con_buf);
        }

        if (_callbackf)
        {
            _callbackf(_con_buf.ReadBuffer());
        }
    }
}
