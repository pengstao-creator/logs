#pragma once
#include <string>
#include <cstddef>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <vector>
#include <cstring>
#include <iostream>
#include <mutex>
#include <functional>
#include <type_traits>
#include "tool.hpp"

#define DConfig ConfigManager::getInstance()::setDefaultConfig()

// X-Macro技术：用于自动添加新配置项
// 用户只需在CONFIG_ITEMS列表中添加一行即可完成新配置项的添加
#define CONFIG_ITEMS(X)                                                                            \
    X(TIME_FORMAT, "log.time_format", "%Y-%m-%d %H:%M:%S", String, {}, "时间格式")                 \
    X(BUFFER_SIZE, "log.buffer_size", "80", SizeT, {}, "缓冲区大小")                               \
    X(MAX_LOGFILE_SIZE, "log.max_logfile_size", "10485760", SizeT, {}, "最大日志文件大小")         \
    X(MAX_BUFFER_SIZE, "log.max_buffer_size", "5242880", SizeT, {}, "最大缓冲区大小")              \
    X(FORMAT, "log.format", "[%L][%N][{%Y-%m-%d %H:%M:%S}][%f][%l][%c]%n", String, {}, "日志格式") \
    X(FILE_TIME_FORMAT, "log.file_time_format", "%Y%m%d%H%M%S", String, {}, "文件时间格式")        \
    X(BASE_FILE_NAME, "log.BaseFileName", "../logs/log", String, {}, "基础文件名")                 \
    X(BOUND_SYMBOL, "log.BoundSymbol", "_", Char, {}, "文件名连接符")                              \
    X(FILE_EXTENSION, "log.file_extension", ".txt", String, {}, "文件扩展名")                      \
    X(LOG_FILE_NAME, "log.logFileName", "", String, {}, "日志文件名")                              \
    X(FILE_SERIAL, "log.file_serial", "0", SizeT, {}, "文件序号")                                  \
    X(LOG_FILE_SIZE, "log.logFileSize", "0", SizeT, {}, "日志文件大小")                            \
    X(MAX_FILE_SERIAL, "log.MaxFileSerial", "50", SizeT, {}, "最大文件序号")                       \
    X(THREAD_COUNT, "log.threadCount", "5", SizeT, {}, "线程数")                                   \
    X(DLOGGER_TYPE, "log.DLoggerType", "ASYNLOGGER", String, {}, "默认日志记录器类型")             \
    X(DANSY_CTRL_TYPE, "log.DAnsyCtrlType", "COMMON", String, {}, "默认异步控制类型")              \
    X(DLEVEL, "log.DLevel", "DEBUG", String, {}, "默认日志级别")

// 声明配置项的宏：展开为枚举值
#define DECLARE_CONFIG_ENUM(Name, Key, DefaultValue, Type, Validator, Description) Name,
#define DEFINE_CONFIG_ITEM(Name, Key, DefaultValue, Type, Validator, Description) {Key, DefaultValue, Type, Validator, Description},

// 声明配置项的宏：自动生成ConfigKey枚举
#define BEGIN_CONFIG_DECLARATION() enum ConfigKey      \
{                                                      \
    CONFIG_ITEMS(DECLARE_CONFIG_ENUM) CONFIG_KEY_COUNT \
};

// 定义配置项的宏：自动生成配置项列表
#define BEGIN_CONFIG_DEFINITION()                                                        \
    static const std::vector<ConfigItem> &configItems()                                  \
    {                                                                                    \
        static const std::vector<ConfigItem> items = {CONFIG_ITEMS(DEFINE_CONFIG_ITEM)}; \
        return items;                                                                    \
    }

// 为不同类型的配置项生成不同的getter方法
#define GENERATE_GETTER_BY_TYPE(Name, Key, DefaultValue, Type, Validator, Description) \
    _GENERATE_GETTER_FOR_##Type(Name)

#define _GENERATE_GETTER_FOR_String(Name)                            \
    const char *get##Name()                                          \
    {                                                                \
        static std::string value = getConfigString(ConfigKey::Name); \
        return value.c_str();                                        \
    }

#define _GENERATE_GETTER_FOR_SizeT(Name)        \
    size_t get##Name() const                    \
    {                                           \
        return getConfigSizeT(ConfigKey::Name); \
    }

#define _GENERATE_GETTER_FOR_Char(Name)        \
    char get##Name() const                     \
    {                                          \
        return getConfigChar(ConfigKey::Name); \
    }

#define _GENERATE_GETTER_FOR_Bool(Name)                        \
    bool get##Name() const                                     \
    {                                                          \
        const auto &item = getConfigItem(ConfigKey::Name);     \
        return getBool(item.key, item.defaultValue == "true"); \
    }

// 为配置项生成GETTER的宏定义
#define GENERATE_GETTERS() \
    CONFIG_ITEMS(GENERATE_GETTER_BY_TYPE)

/*
    管理Data中常用数据的类
*/
namespace Log
{
    // 配置项类型枚举
    enum ConfigType
    {
        String,
        SizeT,
        Char,
        Bool
    };

    // 泛型模板：类型到配置类型的映射
    template <typename T>
    struct ConfigTypeMap;
    template <>
    struct ConfigTypeMap<std::string>
    {
        static constexpr ConfigType value = String;
    };
    template <>
    struct ConfigTypeMap<size_t>
    {
        static constexpr ConfigType value = SizeT;
    };
    template <>
    struct ConfigTypeMap<char>
    {
        static constexpr ConfigType value = Char;
    };
    template <>
    struct ConfigTypeMap<bool>
    {
        static constexpr ConfigType value = Bool;
    };

    // 泛型模板：字符串转换
    template <typename T>
    struct StringConverter;
    template <>
    struct StringConverter<std::string>
    {
        static std::string to(const std::string &value) { return value; }
        static std::string from(const std::string &value) { return value; }
    };
    template <>
    struct StringConverter<size_t>
    {
        static std::string to(size_t value) { return std::to_string(value); }
        static size_t from(const std::string &value) { return std::stoull(value); }
    };
    template <>
    struct StringConverter<char>
    {
        static std::string to(char value) { return std::string(1, value); }
        static char from(const std::string &value) { return value.empty() ? '\0' : value[0]; }
    };
    template <>
    struct StringConverter<bool>
    {
        static std::string to(bool value) { return value ? "true" : "false"; }
        static bool from(const std::string &value)
        {
            return value == "true" || value == "1" || value == "TRUE" || value == "YES" || value == "yes";
        }
    };

    // 配置管理器类（内部实现）
    class ConfigManager
    {
    public:
        // 配置项验证器类型
        using Validator = std::function<bool(const std::string &)>;

        // 配置项结构体
        struct ConfigItem
        {
            std::string key;
            std::string defaultValue;
            ConfigType type;
            Validator validator;
            std::string description;
        };

        // 配置项键枚举 - 使用X-Macro自动生成
        BEGIN_CONFIG_DECLARATION()

    private:
        std::unordered_map<std::string, std::string> _configMap;
        std::mutex _mutex;
        bool _loaded = false;

        static constexpr const char *PropertiesName = "../config/.properties";

        // 配置项定义 - 使用X-Macro自动生成
        BEGIN_CONFIG_DEFINITION()

        ConfigManager() = default;
        ~ConfigManager() = default;
        ConfigManager(const ConfigManager &) = delete;
        ConfigManager &operator=(const ConfigManager &) = delete;

    public:
        static ConfigManager &getInstance()
        {
            static ConfigManager instance;
            return instance;
        }
        bool loadConfig(const std::string &filename)
        {
            if (_loaded)
                return true;

            std::ifstream file(filename);
            if (!file.is_open())
            {
                {
                    std::unique_lock<std::mutex> lock(_mutex);
                    setDefaultConfig();
                    writeDefaultConfig();
                }

                _loaded = true;
                return false;
            }

            std::string line;
            while (std::getline(file, line))
            {
                if (line.empty() || line[0] == '#')
                    continue;

                size_t pos = line.find('=');
                if (pos != std::string::npos)
                {
                    std::string key = trim(line.substr(0, pos));
                    std::string value = trim(line.substr(pos + 1));

                    if (!key.empty() && !value.empty())
                    {
                        _configMap[key] = value;
                    }
                }
            }

            file.close();
            _loaded = true;
            return true;
        }

        std::string getString(const std::string &key, const std::string &defaultValue = "") const
        {
            auto it = _configMap.find(key);
            if (it != _configMap.end())
                return it->second;
            return defaultValue;
        }

        size_t getSizeT(const std::string &key, size_t defaultValue = 0) const
        {
            auto it = _configMap.find(key);
            if (it != _configMap.end())
            {
                try
                {
                    return static_cast<size_t>(std::stoull(it->second));
                }
                catch (...)
                {
                    return defaultValue;
                }
            }
            return defaultValue;
        }

        char getChar(const std::string &key, char defaultValue = '\0') const
        {
            auto it = _configMap.find(key);
            if (it != _configMap.end() && !it->second.empty())
            {
                return it->second[0];
            }
            return defaultValue;
        }

        bool getBool(const std::string &key, bool defaultValue = false) const
        {
            auto it = _configMap.find(key);
            if (it != _configMap.end())
            {
                const std::string &value = it->second;
                return value == "true" || value == "1" || value == "TRUE" || value == "YES" || value == "yes";
            }
            return defaultValue;
        }

        // 自动生成所有配置项的getter方法
        GENERATE_GETTERS()

        bool isLoaded() const { return _loaded; }
       
    private:
        void setDefaultConfig()
        {
            // 使用集中管理的配置项设置默认值
            for (const auto &item : configItems())
            {
                _configMap[item.key] = item.defaultValue;
            }
        }

        // 类型安全访问方法
        const ConfigItem &getConfigItem(ConfigKey key) const
        {
            return configItems()[key];
        }

        std::string getConfigString(ConfigKey key) const
        {
            const auto &item = getConfigItem(key);
            return getString(item.key, item.defaultValue);
        }

        size_t getConfigSizeT(ConfigKey key) const
        {
            const auto &item = getConfigItem(key);
            return getSizeT(item.key, std::stoull(item.defaultValue));
        }

        char getConfigChar(ConfigKey key) const
        {
            const auto &item = getConfigItem(key);
            return getChar(item.key, item.defaultValue[0]);
        }

        bool openfile(const std::string &filename, std::ofstream &file)
        {
            if (!tool::File::FileisExist(filename))
            {
                tool::File::createFilePath(tool::File::GetFilepath(filename));
            }
            file.open(filename, std::ofstream::ate);
            if (!file.is_open())
                return false;
            return true;
        }

        void writeDefaultConfig()
        {
            std::ofstream file;
            if (openfile(PropertiesName, file))
            {
                writeDefaultConfig(file);
                file.close();
            }
        }

        static bool writeDefaultConfig(std::ostream &file)
        {
            // 使用默认值常量
            file << "# 日志配置文件\n\n";

            // 遍历所有配置项，自动生成默认配置
            for (const auto &item : configItems())
            {
                // 跳过特殊的日志文件运行时配置项（这些由系统自动管理）
                if (item.key == "log.logFileName" || item.key == "log.file_serial" || item.key == "log.logFileSize")
                {
                    continue;
                }

                // 输出配置项的描述和默认值
                file << "# " << item.description << "\n";
                file << item.key << "=" << getInstance()._configMap[item.key] << "\n\n";
            }

            return true;
        }

        std::string trim(const std::string &str) const
        {
            size_t first = str.find_first_not_of(" \t\n\r");
            if (first == std::string::npos)
                return "";

            size_t last = str.find_last_not_of(" \t\n\r");
            return str.substr(first, last - first + 1);
        }
    };

}