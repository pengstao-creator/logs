#pragma once
#include "ansyctrl.hpp"
#include "format.hpp"
#include "level.hpp"
#include "message.hpp"
#include "sink.hpp"
#include "ParseFormat.hpp"
#include <atomic>
#include <cstdarg>
#include <mutex>
#include <thread>
#include <vector>
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
      Logger(const LogLevel::VALUE &value, const Data::LogGerType &loggertype,
             const VSPtr &vsptr, const FPtr &fptr, const std::string &loggername);
      virtual ~Logger() {}
      const std::string &GetLoggerName() const { return _loggername; }

      template <class... Args>
      void Debug(int line, const std::string &filename, std::string format,
                 Args... args)
      {
        if (LogLevel::DEBUG < _value || format.empty())
          return;
        
        std::string fmt = _parseformat->parse(format, std::forward<Args>(args)...);
        msgFLog(line, LogLevel::DEBUG, filename, fmt);
      }

      template <class... Args>
      void Info(int line, const std::string &filename, std::string format,
                Args... args)
      {
        if (LogLevel::INFO < _value || format.empty())
          return;
        
        std::string fmt = _parseformat->parse(format, std::forward<Args>(args)...);
        msgFLog(line, LogLevel::INFO, filename, fmt);
      }

      template <class... Args>
      void Warning(int line, const std::string &filename, std::string format,
                   Args... args)
      {
        if (LogLevel::WARNING < _value || format.empty())
          return;
        
        std::string fmt = _parseformat->parse(format, std::forward<Args>(args)...);
        msgFLog(line, LogLevel::WARNING, filename, fmt);
      }

      template <class... Args>
      void Errno(int line, const std::string &filename, std::string format,
                 Args... args)
      {
        if (LogLevel::ERRNO < _value || format.empty())
          return;
        
        std::string fmt = _parseformat->parse(format, std::forward<Args>(args)...);
        msgFLog(line, LogLevel::ERRNO, filename, fmt);
      }

      template <class... Args>
      void Fatal(int line, const std::string &filename, std::string format,
                 Args... args)
      {
        if (LogLevel::FATAL < _value || format.empty())
          return;
        
        std::string fmt = _parseformat->parse(format, std::forward<Args>(args)...);
        msgFLog(line, LogLevel::FATAL, filename, fmt);
      }
      const VSPtr getSink() const { return _vsptr; }

    private:
      void msgFLog(int line, const LogLevel::VALUE &value,
                   const std::string &filename, const std::string &con);

    protected:
      std::atomic<LogLevel::VALUE> _value;
      Data::LogGerType _loggertype;
      std::string _loggername;
      std::shared_ptr<ParseFormat> _parseformat;
      VSPtr _vsptr;
      FPtr _fptr;
      std::mutex _mutex;
    };

    class SyncLogger : public Logger
    {
    public:
      SyncLogger(const LogLevel::VALUE &value, const Data::LogGerType &loggertype,
                 const VSPtr &vsptr, const FPtr &fptr,
                 const std::string &loggername);
      void log(const std::string &str) override;
    };

    class AnsyLogger : public Logger
    {
    public:
      AnsyLogger(const LogLevel::VALUE &value, const Data::LogGerType &loggertype,
                 const VSPtr &vsptr, const FPtr &fptr,
                 const std::string &loggername,
                 const ACtrl::AnsyCtrl::ptr &ansyctrl);
      ~AnsyLogger() override {}
      void log(const std::string &str) override;
      void AnsySink(const std::string &buf);

    private:
      ACtrl::AnsyCtrl::ptr _ansyctrl;
    };

    class LoggerBuilder
    {
    public:
      typedef std::shared_ptr<LoggerBuilder> ptr;
      void InitLevel(LogLevel::VALUE value);
      void InitACType(const Data::AnsyCtrlType &type);
      void InitLoggerType(const Data::LogGerType &type);
      void InitLoggername(const std::string &loggername);
      void InitAnsyCtrlWay(ACtrl::AnsyCtrl::ptr ansyctrl);
      void InitSinkWay(const Logger::VSPtr &vsptr);
      void InitSinkWay(Sink::ptr sptr);
      void InitFormat(const std::string &format);

      virtual Logger::ptr InitLB() = 0;

    protected:
      bool isTypeTrue();
      LogGer::Logger::ptr Initnullptr();

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
      Logger::ptr InitLB() override;
    };
    class SingleManage
    {
    public:
      static SingleManage &getInstance();
      void addLogger(LogGer::Logger::ptr logger);
      LogGer::Logger::ptr getLoger(const std::string &loggername = "");
      LogGer::Logger::ptr DefaultAsynLogger();
      LogGer::Logger::ptr DefaultSyncLogger();
      bool hasLogger(const std::string &name);

    private:
      SingleManage() {}
      ~SingleManage() {}
      LogGer::Logger::ptr DefaultLogger(const std::string &loggername);
      LogGer::Logger::ptr returnLogger(LogGer::LoggerBuilder::ptr &bp,
                   const Data::LogGerType &loggertype = Data::DLoggerType(),
                   const LogLevel::VALUE &value = Data::DLevel(),
                   const std::string &loggername = Data::ASYN,
                   const std::string &format = Data::defaultformat(),
                   const Data::AnsyCtrlType &type = Data::DAnsyCtrlType());

    private:
      std::mutex _mutex;
      std::unordered_map<std::string, LogGer::Logger::ptr> _logger_map;
    };

    class GlobalLogder : public LoggerBuilder
    {
    public:
      Logger::ptr InitLB() override;
    };

  } // namespace LogGer

  class Director
  {
  public:
    LogGer::Logger::ptr LocalLogder(const std::string &loggername = Data::ASYN,
                const Data::LogGerType &loggertype = Data::DLoggerType(),
                const LogLevel::VALUE &value = Data::DLevel(),
                const std::string &format = Data::defaultformat(),
                const Data::AnsyCtrlType &type = Data::DAnsyCtrlType());

    LogGer::Logger::ptr GlobalLogder(const std::string &loggername = Data::ASYN,
                 const Data::LogGerType &loggertype = Data::DLoggerType(),
                 const LogLevel::VALUE &value = Data::DLevel(),
                 const std::string &format = Data::defaultformat(),
                 const Data::AnsyCtrlType &type = Data::DAnsyCtrlType());

    template <class SW, class... Args>
    void AddSink(Args &&...args)
    {
      Sink::ptr sp = SinkFactory::SinkWay<SW>(std::forward<Args>(args)...);
      _vsptr.push_back(sp);
    }

    template <class AnsyWay, class... Args>
    void AddAnsyWay(Args &&...args)
    {
      ACtrl::AnsyCtrl::ptr apr =
          ACtrlFactory::ACtrlWay<AnsyWay>(std::forward<Args>(args)...);
      _ansyctrl = apr;
    }

  private:
    LogGer::Logger::ptr returnLogger(LogGer::LoggerBuilder::ptr &bp,
                 const Data::LogGerType &loggertype = Data::DLoggerType(),
                 const LogLevel::VALUE &value = Data::DLevel(),
                 const std::string &loggername = Data::ASYN,
                 const std::string &format = Data::defaultformat(),
                 const Data::AnsyCtrlType &type = Data::DAnsyCtrlType());

  private:
    LogGer::Logger::VSPtr _vsptr;
    ACtrl::AnsyCtrl::ptr _ansyctrl;
  };

} // namespace Log
