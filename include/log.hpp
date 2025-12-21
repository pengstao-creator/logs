#pragma once
#include "logger.hpp"
/*
    用户使用只需要包含次头文件
    使用宏函数简化用户的操作
*/

namespace mylog
{
    void AddLogger(Log::LogGer::Logger::ptr logger)
    {
        return Log::LogGer::SingleManage::getInstance().addLogger(logger);
    }

    Log::LogGer::Logger::ptr GetLogger(const std::string &loggername)
    {
        return Log::LogGer::SingleManage::getInstance().getLoger(loggername);
    }
    Log::LogGer::Logger::ptr DefaultAsynLogger()
    {
        return Log::LogGer::SingleManage::getInstance().DefaultAsynLogger();
    }

    Log::LogGer::Logger::ptr DefaultSyncLogger()
    {
        return Log::LogGer::SingleManage::getInstance().DefaultSyncLogger();
    }

#define DEBUG(fmt, ...) Debug(__LINE__, __FILE__, fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) Info(__LINE__, __FILE__, fmt, ##__VA_ARGS__)
#define WARNING(fmt, ...) Warning(__LINE__, __FILE__, fmt, ##__VA_ARGS__)
#define ERRNO(fmt, ...) Errno(__LINE__, __FILE__, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) Fatal(__LINE__, __FILE__, fmt, ##__VA_ARGS__)

#define DEBUG_A(...) mylog::DefaultAsynLogger()->DEBUG(__VA_ARGS__)
#define INFO_A(...) mylog::DefaultAsynLogger()->INFO(__VA_ARGS__)
#define WARNING_A(...) mylog::DefaultAsynLogger()->WARNING(__VA_ARGS__)
#define ERRNO_A(...) mylog::DefaultAsynLogger()->ERRNO(__VA_ARGS__)
#define FATAL_A(...) mylog::DefaultAsynLogger()->FATAL(__VA_ARGS__)

#define DEBUG_S(...) mylog::DefaultSyncLogger()->DEBUG(__VA_ARGS__)
#define INFO_S(...) mylog::DefaultSyncLogger()->INFO(__VA_ARGS__)
#define WARNING_S(...) mylog::DefaultSyncLogger()->WARNING(__VA_ARGS__)
#define ERRNO_S(...) mylog::DefaultSyncLogger()->ERRNO(__VA_ARGS__)
#define FATAL_S(...) mylog::DefaultSyncLogger()->FATAL(__VA_ARGS__)

}
