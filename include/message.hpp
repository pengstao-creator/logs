#pragma once
#include <thread>
#include "tool.hpp"
#include "level.hpp"
#include "logdata.hpp"
// 日志消息管理模块
// 1.日志产生时间
// 2.日志等级
// 3.日志有效内容
// 4.文件名称
// 5.线程id
// 6.错误行号
// 7.日志器名称

namespace Log
{
  class Message
  {
  public:
    time_t _time;
    int _line;
    Log::LogLevel::VALUE _value;
    std::thread::id _tid;
    std::string _filename;
    Log::Data::LogGerType _loggertype;
    std::string _loggername;
    std::string _content;

    Message(int line, Log::LogLevel::VALUE value,
            const std::string &filename,
            const Log::Data::LogGerType &loggertype,
            const std::string &loggername,
            const std::string &content)
        : _time(Log::tool::Date::GetTime()),
          _line(line), _value(value),
          _tid(std::this_thread::get_id()),
          _filename(filename), _loggertype(loggertype),
          _loggername(loggername),_content(content)
    {
    }
  };

}