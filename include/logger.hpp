#pragma once
#include "format.hpp"
#include "level.hpp"
#include "sink.hpp"
#include "message.hpp"
#include "ansyctrl.hpp"
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>
#include <cstdarg>
/*
    日志器模块
    1.对其他模块的整合
    2.同步日志器
    3.异步日志器
    4.通过建造这模式返回对象
    5.局部日志
    6.全局日志
*/

namespace Log
{
    namespace LogGer
    {

        class Logger
        {
        public:
            typedef std::vector<Sink::ptr> VSPtr;
            typedef Format::FormatBase::ptr FPtr;
            typedef std::shared_ptr<Logger> ptr;

        protected:
            virtual void log(const std::string &) = 0;

        public:
            Logger(const LogLevel::VALUE &value,
                   const Data::LogGerType &loggertype,
                   const VSPtr &vsptr, const FPtr &fptr,
                   const std::string &loggername)
                : _value(value), _loggertype(loggertype), _vsptr(vsptr.begin(), vsptr.end()), _fptr(fptr), _loggername(loggername)
            {
            }
            virtual ~Logger() {}
            const std::string &GetLoggerName() const { return _loggername; }
            void Debug(int line, const std::string &filename, const std::string &format, ...)
            {
                Debug(line, filename, format.c_str());
            }
            //处理将format传成nullptr
            void Debug(int line, const std::string &filename, const char *format, ...)
            {
                if (LogLevel::DEBUG < _value)
                    return;
                va_list args;
                va_start(args, format);
                char *ret;
                const char *fmt = format ? format : "";
                int n = vasprintf(&ret, fmt, args);
                if (n < 0)
                {
                    perror("vasprintf");
                    ret = nullptr;
                }
                va_end(args);
                msgFLog(line, LogLevel::DEBUG, filename, ret ? std::string(ret) : std::string());
                if (ret)
                    free(ret);
            }

            void Info(int line, const std::string &filename, const std::string &format, ...)
            {
                Info(line, filename, format.c_str());
            }
            void Info(int line, const std::string &filename, const char *format, ...)
            {
                if (LogLevel::INFO < _value)
                    return;
                va_list args;
                va_start(args, format);
                char *ret;
                const char *fmt = format ? format : "";
                int n = vasprintf(&ret, fmt, args);
                if (n < 0)
                {
                    perror("vasprintf");
                    ret = nullptr;
                }
                va_end(args);
                msgFLog(line, LogLevel::INFO, filename, ret ? std::string(ret) : std::string());
                if (ret)
                    free(ret);
            }
            void Errno(int line, const std::string &filename, const std::string &format, ...)
            {
                Errno(line, filename, format.c_str());
            }
            void Errno(int line, const std::string &filename, const char *format, ...)
            {
                if (LogLevel::ERRNO < _value)
                    return;
                va_list args;
                va_start(args, format);
                char *ret;
                const char *fmt = format ? format : "";
                int n = vasprintf(&ret, fmt, args);
                if (n < 0)
                {
                    perror("vasprintf");
                    ret = nullptr;
                }
                va_end(args);
                msgFLog(line, LogLevel::ERRNO, filename, ret ? std::string(ret) : std::string());
                if (ret)
                    free(ret);
            }
            void Warning(int line, const std::string &filename, const std::string &format, ...)
            {
                Warning(line, filename, format.c_str());
            }
            void Warning(int line, const std::string &filename, const char *format, ...)
            {
                if (LogLevel::WARNING < _value)
                    return;
                va_list args;
                va_start(args, format);
                char *ret;
                const char *fmt = format ? format : "";
                int n = vasprintf(&ret, fmt, args);
                if (n < 0)
                {
                    perror("vasprintf");
                    ret = nullptr;
                }
                va_end(args);
                msgFLog(line, LogLevel::WARNING, filename, ret ? std::string(ret) : std::string());
                if (ret)
                    free(ret);
            }
            void Fatal(int line, const std::string &filename, const std::string &format, ...)
            {
                Fatal(line, filename, format.c_str());
            }
            void Fatal(int line, const std::string &filename, const char *format, ...)
            {
                if (LogLevel::FATAL < _value)
                    return;
                va_list args;
                va_start(args, format);
                char *ret;
                const char *fmt = format ? format : "";
                int n = vasprintf(&ret, fmt, args);
                if (n < 0)
                {
                    perror("vasprintf");
                    ret = nullptr;
                }
                va_end(args);
                msgFLog(line, LogLevel::FATAL, filename, ret ? std::string(ret) : std::string());
                if (ret)
                    free(ret);
            }
            const VSPtr getSink() const { return _vsptr; }

        private:
            void msgFLog(int line, const LogLevel::VALUE &value, const std::string &filename, const std::string &con)
            {
                Message msg(line, value, filename, _loggertype, _loggername, con);
                std::stringstream ss;
                Formatctrl fc;
                log(fc.format(msg));
            }

        protected:
            std::atomic<LogLevel::VALUE> _value;
            Data::LogGerType _loggertype;
            std::string _loggername;
            VSPtr _vsptr;
            FPtr _fptr;
            std::mutex _mutex;
        };

        class SyncLogger : public Logger
        {
        public:
            SyncLogger(const LogLevel::VALUE &value,
                       const Data::LogGerType &loggertype,
                       const VSPtr &vsptr, const FPtr &fptr,
                       const std::string &loggername)
                : Logger(value, loggertype, vsptr, fptr, loggername)
            {
            }
            void log(const std::string &str) override
            {
                std::unique_lock<std::mutex> lock(_mutex);
                for (auto &sink : _vsptr)
                {
                    sink->WriteFile(str);
                }
            }
        };

        class AnsyLogger : public Logger
        {
        public:
            AnsyLogger(const LogLevel::VALUE &value,
                       const Data::LogGerType &loggertype,
                       const VSPtr &vsptr, const FPtr &fptr,
                       const std::string &loggername,
                       const ACtrl::AnsyCtrl::ptr &ansyctrl)
                : Logger(value, loggertype, vsptr, fptr, loggername), _ansyctrl(ansyctrl)
            {
                _ansyctrl->bindcallbackf(std::bind(
                    &AnsyLogger::AnsySink, this, std::placeholders::_1));
            }
            ~AnsyLogger() override {}
            void log(const std::string &str) override
            {
                _ansyctrl->push(str);
            }
            void AnsySink(const std::string &buf)
            {
                std::unique_lock<std::mutex> lock(_mutex);
                for (auto &sink : _vsptr)
                {
                    sink->WriteFile(buf);
                }
            }

        private:
            ACtrl::AnsyCtrl::ptr _ansyctrl;
        };

        class LoggerBuilder
        {
        public:
            typedef std::shared_ptr<LoggerBuilder> ptr;
            void InitLevel(LogLevel::VALUE value) { _value = value; }
            void InitACType(const Data::AnsyCtrlType &type) { _ACType = type; }
            void InitLoggerType(const Data::LogGerType &type) { _loggertype = type; }
            void InitLoggername(const std::string &loggername) { _loggername = loggername; }
            void InitAnsyCtrlWay(ACtrl::AnsyCtrl::ptr ansyctrl) { _ansyctrl = ansyctrl; }
            void InitSinkWay(const Logger::VSPtr &vsptr)
            {
                _vsptr = vsptr;
            }
            void InitSinkWay(Sink::ptr sptr)
            {
                _vsptr.push_back(sptr);
            }
            void InitFormat(const std::string &format)
            {
                _fptr = std::make_shared<Formatctrl>(format);
            }

            virtual Logger::ptr InitLB() = 0;

        protected:
            bool isTypeTrue()
            {
                if (_loggertype == Data::ASYNLOGGER || _loggertype == Data::SYNCLOGGER)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            LogGer::Logger::ptr Initnullptr()
            {
                if (_vsptr.empty())
                {
                    _vsptr.push_back(SinkFactory::RollFileSink());
                }

                if (!isTypeTrue())
                    _loggertype = Data::ASYNLOGGER;

                if (!_ansyctrl)
                {
                    if (_ACType == Data::AnsyCtrlType::THPOOL && _loggertype == Data::ASYNLOGGER)
                        _ansyctrl = ACtrlFactory::AnsyThpool();
                    else
                        _ansyctrl = ACtrlFactory::AnsyCommon();
                }

                if (_loggertype == Data::ASYNLOGGER)
                {
                    _loggertype = Data::ASYNLOGGER;
                    return std::make_shared<LogGer::AnsyLogger>(_value, _loggertype, _vsptr, _fptr, _loggername, _ansyctrl);
                }
                else
                {
                    return std::make_shared<LogGer::SyncLogger>(_value, _loggertype, _vsptr, _fptr, _loggername);
                }
            }

        protected:
            LogLevel::VALUE _value;
            Data::AnsyCtrlType _ACType;
            Data::LogGerType _loggertype;
            std::string _loggername;
            Logger::VSPtr _vsptr;
            Logger::FPtr _fptr;
            ACtrl::AnsyCtrl::ptr _ansyctrl;
        };

        class LocalLogder : public LoggerBuilder
        {
        public:
            Logger::ptr InitLB() override
            {

                return Initnullptr();
            }
        };
        class SingleManage
        {
        public:
            static SingleManage &getInstance()
            {
                static SingleManage ince;
                return ince;
            }
            void addLogger(LogGer::Logger::ptr logger)
            {
                const std::string &loggername = logger->GetLoggerName();
                if (hasLogger(loggername))
                    return;
                std::unique_lock<std::mutex> lock(_mutex);
                _logger_map.insert({loggername, logger});
            }

            LogGer::Logger::ptr getLoger(const std::string &loggername = "")
            {
                if (!hasLogger(loggername))
                {
                    return nullptr;
                }
                return _logger_map[loggername];
            }

            LogGer::Logger::ptr DefaultAsynLogger()
            {
                return _logger_map[Data::ASYN];
            }
            LogGer::Logger::ptr DefaultSyncLogger()
            {
                return _logger_map[Data::SYNC];
            }

            bool hasLogger(const std::string &name)
            {
                std::unique_lock<std::mutex> lock(_mutex);
                auto it = _logger_map.find(name);
                if (it == _logger_map.end())
                    return false;
                return true;
            }

        private:
            SingleManage()
            {
                const std::string &loggernameA = Data::ASYN;
                const std::string &loggernameS = Data::SYNC;
                DefaultLogger(loggernameA);
                DefaultLogger(loggernameS);
            }

            ~SingleManage()
            {
            }
            LogGer::Logger::ptr DefaultLogger(const std::string &loggername)
            {
                if (hasLogger(loggername))
                {
                    return _logger_map[loggername];
                }
                LogGer::Logger::ptr lg;
                if (loggername == Data::ASYN)
                {
                    LogGer::LoggerBuilder::ptr bp = std::make_shared<LogGer::LocalLogder>();
                    lg = returnLogger(bp);
                }
                else
                {
                    LogGer::LoggerBuilder::ptr bp = std::make_shared<LogGer::LocalLogder>();
                    //同步日志器默认向屏幕打印
                    bp->InitSinkWay(SinkFactory::StdoutSink());
                    lg = returnLogger(bp, Data::LogGerType::SYNCLOGGER, Data::DLevel(), Data::SYNC);
                }
                addLogger(lg);
                return lg;
            }
            LogGer::Logger::ptr returnLogger(LogGer::LoggerBuilder::ptr &bp,
                                             const Data::LogGerType &loggertype = Data::DLoggerType(),
                                             const LogLevel::VALUE &value = Data::DLevel(),
                                             const std::string &loggername = Data::ASYN,
                                             const std::string &format = Data::defaultformat(),
                                             const Data::AnsyCtrlType &type = Data::DAnsyCtrlType())
            {
                bp->InitLevel(value);
                bp->InitACType(type);
                bp->InitLoggerType(loggertype);
                bp->InitLoggername(loggername);
                bp->InitFormat(format);
                return bp->InitLB();
            }

        private:
            std::mutex _mutex;
            std::unordered_map<std::string, LogGer::Logger::ptr> _logger_map;
        };

        class GlobalLogder : public LoggerBuilder
        {
        public:
            Logger::ptr InitLB() override
            {
                // 在全局日志器中存在则直接返回
                if (SingleManage::getInstance().hasLogger(_loggername))
                {
                    return SingleManage::getInstance().getLoger(_loggername);
                }
                Logger::ptr logger = Initnullptr();
                SingleManage::getInstance().addLogger(logger);
                return logger;
            }
        };

    }

    class Director
    {
    public:
        LogGer::Logger::ptr LocalLogder(const std::string &loggername = Data::ASYN,
                                        const Data::LogGerType &loggertype = Data::DLoggerType(),
                                        const LogLevel::VALUE &value = Data::DLevel(),
                                        const std::string &format = Data::defaultformat(),
                                        const Data::AnsyCtrlType &type = Data::DAnsyCtrlType())
        {
            LogGer::LoggerBuilder::ptr bp = std::make_shared<LogGer::LocalLogder>();
            return returnLogger(bp, loggertype, value, loggername, format, type);
        }

        LogGer::Logger::ptr GlobalLogder(const std::string &loggername = Data::ASYN,
                                         const Data::LogGerType &loggertype = Data::DLoggerType(),
                                         const LogLevel::VALUE &value = Data::DLevel(),
                                         const std::string &format = Data::defaultformat(),
                                         const Data::AnsyCtrlType &type = Data::DAnsyCtrlType())
        {
            LogGer::LoggerBuilder::ptr bp = std::make_shared<LogGer::GlobalLogder>();
            return returnLogger(bp, loggertype, value, loggername, format, type);
        }

        template <class SW, class... Args>
        void AddSink(Args &&...args)
        {
            Sink::ptr sp = SinkFactory::SinkWay<SW>(std::forward<Args>(args)...);
            _vsptr.push_back(sp);
        }

        template <class AnsyWay, class... Args>
        void AddAnsyWay(Args &&...args)
        {
            ACtrl::AnsyCtrl::ptr apr = ACtrlFactory::ACtrlWay<AnsyWay>(std::forward<Args>(args)...);
            _ansyctrl = apr;
        }

    private:
        LogGer::Logger::ptr returnLogger(LogGer::LoggerBuilder::ptr &bp,
                                         const Data::LogGerType &loggertype = Data::DLoggerType(),
                                         const LogLevel::VALUE &value = Data::DLevel(),
                                         const std::string &loggername = Data::ASYN,
                                         const std::string &format = Data::defaultformat(),
                                         const Data::AnsyCtrlType &type = Data::DAnsyCtrlType())
        {
            bp->InitLevel(value);
            bp->InitACType(type);
            bp->InitLoggerType(loggertype);
            bp->InitLoggername(loggername);
            bp->InitFormat(format);
            bp->InitSinkWay(_vsptr);
            bp->InitAnsyCtrlWay(_ansyctrl);
            return bp->InitLB();
        }

    private:
        LogGer::Logger::VSPtr _vsptr;
        ACtrl::AnsyCtrl::ptr _ansyctrl;
    };

}
