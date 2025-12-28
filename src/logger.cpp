#include "../include/pch.hpp"
#include "../include/logger.hpp"
#include "../include/message.hpp"
#include <sstream>

namespace Log
{
    LogGer::Logger::Logger(const LogLevel::VALUE &value, const Data::LogGerType &loggertype,
             const VSPtr &vsptr, const FPtr &fptr, const std::string &loggername)
        : _value(value), _loggertype(loggertype),
          _vsptr(vsptr.begin(), vsptr.end()), _fptr(fptr),
          _loggername(loggername), _parseformat(std::make_shared<ParseFormat>()) {}

    void LogGer::Logger::msgFLog(int line, const LogLevel::VALUE &value,
                   const std::string &filename, const std::string &con)
    {
        Message msg(line, value, filename, _loggertype, _loggername, con);
        std::stringstream ss;
        Formatctrl fc;
        log(fc.format(msg));
    }

    LogGer::SyncLogger::SyncLogger(const LogLevel::VALUE &value, const Data::LogGerType &loggertype,
                 const VSPtr &vsptr, const FPtr &fptr, const std::string &loggername)
        : Logger(value, loggertype, vsptr, fptr, loggername) {}

    void LogGer::SyncLogger::log(const std::string &str)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for (auto &sink : _vsptr)
        {
            sink->WriteFile(str);
        }
    }

    LogGer::AnsyLogger::AnsyLogger(const LogLevel::VALUE &value, const Data::LogGerType &loggertype,
                 const VSPtr &vsptr, const FPtr &fptr,
                 const std::string &loggername, const ACtrl::AnsyCtrl::ptr &ansyctrl)
        : Logger(value, loggertype, vsptr, fptr, loggername), _ansyctrl(ansyctrl) 
        {
            _ansyctrl->bindcallbackf([this](const std::string &buf) {
                AnsySink(buf);
            });
        }

    void LogGer::AnsyLogger::log(const std::string &str)
    {
        _ansyctrl->push(str);
    }

    void LogGer::AnsyLogger::AnsySink(const std::string &buf)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        for (auto &sink : _vsptr)
        {
            sink->WriteFile(buf);
        }
    }

    void LogGer::LoggerBuilder::InitLevel(LogLevel::VALUE value) { _value = value; }

    void LogGer::LoggerBuilder::InitACType(const Data::AnsyCtrlType &type) { _ACType = type; }

    void LogGer::LoggerBuilder::InitLoggerType(const Data::LogGerType &type) { _loggertype = type; }

    void LogGer::LoggerBuilder::InitLoggername(const std::string &loggername) { _loggername = loggername; }

    void LogGer::LoggerBuilder::InitAnsyCtrlWay(ACtrl::AnsyCtrl::ptr ansyctrl) { _ansyctrl = ansyctrl; }

    void LogGer::LoggerBuilder::InitSinkWay(const Logger::VSPtr &vsptr) { _vsptr = vsptr; }

    void LogGer::LoggerBuilder::InitSinkWay(Sink::ptr sptr) { _vsptr.push_back(sptr); }

    void LogGer::LoggerBuilder::InitFormat(const std::string &format) { _fptr = std::make_shared<Formatctrl>(format); }

    bool LogGer::LoggerBuilder::isTypeTrue()
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

    LogGer::Logger::ptr LogGer::LoggerBuilder::Initnullptr()
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

    LogGer::Logger::ptr LogGer::LocalLogder::InitLB() { return Initnullptr(); }

    LogGer::SingleManage &LogGer::SingleManage::getInstance()
    {
        static SingleManage ince;
        return ince;
    }

    void LogGer::SingleManage::addLogger(LogGer::Logger::ptr logger)
    {
        const std::string &loggername = logger->GetLoggerName();
        if (hasLogger(loggername))
            return;
        std::unique_lock<std::mutex> lock(_mutex);
        _logger_map.insert({loggername, logger});
    }

    LogGer::Logger::ptr LogGer::SingleManage::getLoger(const std::string &loggername)
    {
        if (loggername == Data::ASYN)
        {
            return DefaultAsynLogger();
        }
        else if (loggername == Data::SYNC)
        {
            return DefaultSyncLogger();
        }

        if (!hasLogger(loggername))
        {
            return nullptr;
        }
        return _logger_map[loggername];
    }

    LogGer::Logger::ptr LogGer::SingleManage::DefaultAsynLogger()
    {
        if (!hasLogger(Data::ASYN))
        {
            const std::string &loggernameA = Data::ASYN;
            DefaultLogger(loggernameA);
        }
        return _logger_map[Data::ASYN];
    }

    LogGer::Logger::ptr LogGer::SingleManage::DefaultSyncLogger()
    {
        if (!hasLogger(Data::SYNC))
        {
            const std::string &loggernameS = Data::SYNC;
            DefaultLogger(loggernameS);
        }
        return _logger_map[Data::SYNC];
    }

    bool LogGer::SingleManage::hasLogger(const std::string &name)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        auto it = _logger_map.find(name);
        if (it == _logger_map.end())
            return false;
        return true;
    }

    LogGer::Logger::ptr LogGer::SingleManage::DefaultLogger(const std::string &loggername)
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
            bp->InitSinkWay(SinkFactory::StdoutSink());
            lg = returnLogger(bp, Data::LogGerType::SYNCLOGGER, Data::DLevel(), Data::SYNC);
        }
        addLogger(lg);
        return lg;
    }

    LogGer::Logger::ptr LogGer::SingleManage::returnLogger(LogGer::LoggerBuilder::ptr &bp,
                                                           const Data::LogGerType &loggertype,
                                                           const LogLevel::VALUE &value,
                                                           const std::string &loggername,
                                                           const std::string &format,
                                                           const Data::AnsyCtrlType &type)
    {
        bp->InitLevel(value);
        bp->InitACType(type);
        bp->InitLoggerType(loggertype);
        bp->InitLoggername(loggername);
        bp->InitFormat(format);
        return bp->InitLB();
    }

    LogGer::Logger::ptr LogGer::GlobalLogder::InitLB()
    {
        if (SingleManage::getInstance().hasLogger(_loggername))
        {
            return SingleManage::getInstance().getLoger(_loggername);
        }
        Logger::ptr logger = Initnullptr();
        SingleManage::getInstance().addLogger(logger);
        return logger;
    }

    LogGer::Logger::ptr Director::LocalLogder(const std::string &loggername,
                                              const Data::LogGerType &loggertype,
                                              const LogLevel::VALUE &value,
                                              const std::string &format,
                                              const Data::AnsyCtrlType &type)
    {
        LogGer::LoggerBuilder::ptr bp = std::make_shared<LogGer::LocalLogder>();
        return returnLogger(bp, loggertype, value, loggername, format, type);
    }

    LogGer::Logger::ptr Director::GlobalLogder(const std::string &loggername,
                                               const Data::LogGerType &loggertype,
                                               const LogLevel::VALUE &value,
                                               const std::string &format,
                                               const Data::AnsyCtrlType &type)
    {
        LogGer::LoggerBuilder::ptr bp = std::make_shared<LogGer::GlobalLogder>();
        return returnLogger(bp, loggertype, value, loggername, format, type);
    }

    LogGer::Logger::ptr Director::returnLogger(LogGer::LoggerBuilder::ptr &bp,
                                               const Data::LogGerType &loggertype,
                                               const LogLevel::VALUE &value,
                                               const std::string &loggername,
                                               const std::string &format,
                                               const Data::AnsyCtrlType &type)
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
}
