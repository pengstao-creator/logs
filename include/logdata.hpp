#pragma once
#include <string>
#include <cstddef>
#include "ConfigManager.hpp"
#include "level.hpp"

/*
    日志系统中常用的常量或者枚举类型
*/
namespace Log
{
    class Data
    {
    private:
        static bool &initialized()
        {
            static bool init = false;
            return init;
        }

        static void ensureInitialized()
        {
            if (!initialized())
            {
                init();
            }
        }

    public:
        static ConfigManager &configManager()
        {
            return ConfigManager::getInstance();
        }
        // 这些可以保持为静态常量
        static constexpr const char *ASYN = "ASYNLOGGER";
        static constexpr const char *SYNC = "SYNCLOGGER";
        static constexpr const char *PropertiesName = "../config/.properties";

        enum LogGerType
        {
            SYNCLOGGER,
            ASYNLOGGER
        };

        enum AnsyCtrlType
        {
            COMMON,
            THPOOL
        };

        static const LogGerType StoLogGerType(const std::string &s)
        {
            if (s == "SYNCLOGGER")
                return SYNCLOGGER;
            else
                return ASYNLOGGER;
        }

        static const AnsyCtrlType StoAnsyCtrlType(const std::string &s)
        {
            if (s == "COMMON")
                return COMMON;
            else
                return THPOOL;
        }
        // 使用静态函数返回配置值
        static const std::string GetFormatTime(const time_t &nowt, const char *format = Data::defaultTF())
        {
            struct tm *t = localtime(&nowt);
            if (t)
            {
                char buffer[Data::size()];
                strftime(buffer, sizeof(buffer), format, t);
                return buffer;
            }
            else
            {
                return "";
            }
        }

        // 确保toString方法存在 - 修复format.hpp中的错误
        static std::string toString(LogGerType lt)
        {
            return lt == SYNCLOGGER ? "SYNCLOGGER" : "ASYNLOGGER";
        }

        // 初始化函数
        static bool init(const std::string &configFile = PropertiesName)
        {
            if (initialized())
            {
                return configManager().isLoaded();
            }

            bool loaded = configManager().loadConfig(configFile);
            initialized() = true;

            return loaded;
        }

// 使用宏自动生成简单的配置项getter方法
// 定义所有简单的配置项
#define DATA_CONFIG_ITEMS(X)                            \
    X(const char *, defaultTF, TIME_FORMAT)             \
    X(const size_t, size, BUFFER_SIZE)                  \
    X(const size_t, max_logfile_size, MAX_LOGFILE_SIZE) \
    X(const size_t, max_buffer_size, MAX_BUFFER_SIZE)   \
    X(const char *, defaultformat, FORMAT)              \
    X(const char *, defaultFileTF, FILE_TIME_FORMAT)    \
    X(const char *, defaultBFile, BASE_FILE_NAME)       \
    X(const char *, defaultFix, FILE_EXTENSION)         \
    X(const char, BoundSymbol, BOUND_SYMBOL)            \
    X(const char *, LogFileName, LOG_FILE_NAME)         \
    X(const size_t, LogFileSerial, FILE_SERIAL)         \
    X(const size_t, LogFileSize, LOG_FILE_SIZE)         \
    X(const size_t, MaxFileSerial, MAX_FILE_SERIAL)     \
    X(const size_t, threadCount, THREAD_COUNT)

// 生成简单getter方法的宏
#define GENERATE_SIMPLE_GETTER(ReturnType, MethodName, ConfigName) \
    static ReturnType MethodName()                                 \
    {                                                              \
        ensureInitialized();                                       \
        return configManager().get##ConfigName();                  \
    }

        // 自动生成所有简单的getter方法
        DATA_CONFIG_ITEMS(GENERATE_SIMPLE_GETTER)

        // 手动实现需要特殊处理的方法
        static const LogGerType DLoggerType()
        {
            ensureInitialized();
            return StoLogGerType(configManager().getDLOGGER_TYPE());
        }
        static const AnsyCtrlType DAnsyCtrlType()
        {
            ensureInitialized();
            return StoAnsyCtrlType(configManager().getDANSY_CTRL_TYPE());
        }
        static const LogLevel::VALUE DLevel()
        {
            ensureInitialized();
            return LogLevel::StoLevel(configManager().getDLEVEL());
        }

// 清理宏定义
#undef DATA_CONFIG_ITEMS
#undef GENERATE_SIMPLE_GETTER
    };

    namespace
    {
        // 如果需要自动初始化，取消注释下面这行
        // bool autoInit = Data::init();
    }
};