#pragma once
#include <iostream>
//
//日志等级模块
//将日志的等级字符串输出
//

namespace Log
{
    class LogLevel
    {
    public:
        enum VALUE
        {
            UNKNOW = 0,
            DEBUG,
            INFO,
            WARNING,
            ERRNO,
            FATAL,
            OFF
        };
        static const char * toString(const VALUE& val)
        {
            switch (val)
            {
            case VALUE::DEBUG : return "DEBUG";
            case VALUE::INFO : return "INFO";
            case VALUE::WARNING : return "WARNING";
            case VALUE::ERRNO : return "ERRNO";
            case VALUE::FATAL : return "FATAL";
            case VALUE::OFF : return "OFF";
            default:
                return "UNKNOW";
            }
       } 

       static const VALUE StoLevel(const std::string& s)
        {
          if(s == "DEBUG") return DEBUG;
          else if(s == "INFO")  return INFO;
          else if(s == "WARNING")  return WARNING;
          else if(s == "ERRNO")  return ERRNO;
          else if(s == "FATAL")  return FATAL;
          else if(s == "OFF")  return OFF;
          else return UNKNOW;
        } 
    };
}