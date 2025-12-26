#pragma once
#include <string>
#include <cstddef>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <type_traits>
#include "tool.hpp"

#define DConfig ConfigManager::getInstance()::setDefaultConfig()

#define CONFIG_ITEMS(X)                                                                             \
    X(TIME_FORMAT, "log.time_format", "%Y-%m-%d %H:%M:%S", String, {}, "时间格式")                  \
    X(MAX_LOGFILE_SIZE, "log.max_logfile_size", "10485760", SizeT, {}, "最大日志文件大小(字节)")    \
    X(EXCEED_SIZE, "log.Exceed_size", "1024", SizeT, {}, "日志文件大小可超过阈值的大小(字节)")      \
    X(MAX_BUFFER_SIZE, "log.max_buffer_size", "1048567", SizeT, {}, "最大缓冲区大小(字节)")         \
    X(FORMAT, "log.format", "[%L][%N][{%Y-%m-%d %H:%M:%S}][%f][%l][%c]%n", String, {}, "日志格式")  \
    X(FILE_TIME_FORMAT, "log.file_time_format", "%Y%m%d%H%M%S", String, {}, "文件时间格式")         \
    X(BASE_FILE_NAME, "log.BaseFileName", "../logs/log", String, {}, "基础文件名")                  \
    X(BOUND_SYMBOL, "log.BoundSymbol", "_", Char, {}, "文件名连接符")                               \
    X(FILE_EXTENSION, "log.file_extension", ".txt", String, {}, "文件扩展名")                       \
    X(MAX_FILE_SERIAL, "log.MaxFileSerial", "50", SizeT, {}, "最大文件序号")                        \
    X(THREAD_COUNT, "log.threadCount", "5", SizeT, {}, "线程数")                                    \
    X(DLOGGER_TYPE, "log.DLoggerType", "ASYNLOGGER", String, {}, "默认日志记录器类型")              \
    X(DANSY_CTRL_TYPE, "log.DAnsyCtrlType", "COMMON", String, {}, "默认异步控制类型 COMMON/THPOOL") \
    X(DLEVEL, "log.DLevel", "DEBUG", String, {}, "默认日志级别")

#define DECLARE_CONFIG_ENUM(Name, Key, DefaultValue, Type, Validator, Description) Name,
#define DEFINE_CONFIG_ITEM(Name, Key, DefaultValue, Type, Validator, Description) {Key, DefaultValue, Type, Validator, Description},

#define BEGIN_CONFIG_DECLARATION() enum ConfigKey      \
{                                                      \
    CONFIG_ITEMS(DECLARE_CONFIG_ENUM) CONFIG_KEY_COUNT \
};

#define BEGIN_CONFIG_DEFINITION()                                                        \
    static const std::vector<ConfigItem> &configItems()                                  \
    {                                                                                    \
        static const std::vector<ConfigItem> items = {CONFIG_ITEMS(DEFINE_CONFIG_ITEM)}; \
        return items;                                                                    \
    }

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

#define GENERATE_GETTERS() \
    CONFIG_ITEMS(GENERATE_GETTER_BY_TYPE)

namespace Log
{
    enum ConfigType
    {
        String,
        SizeT,
        Char,
        Bool
    };

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

    class ConfigManager
    {
    public:
        using Validator = std::function<bool(const std::string &)>;

        struct ConfigItem
        {
            std::string key;
            std::string defaultValue;
            ConfigType type;
            Validator validator;
            std::string description;
        };

        BEGIN_CONFIG_DECLARATION()

    private:
        std::unordered_map<std::string, std::string> _configMap;
        mutable std::mutex _mutex;
        bool _loaded = false;

        static constexpr const char *PropertiesName = "../config/.properties";

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

        bool loadConfig(const std::string &filename);
        std::string getString(const std::string &key, const std::string &defaultValue = "") const;
        size_t getSizeT(const std::string &key, size_t defaultValue = 0) const;
        char getChar(const std::string &key, char defaultValue = '\0') const;
        bool getBool(const std::string &key, bool defaultValue = false) const;
        GENERATE_GETTERS()
        bool isLoaded() const { return _loaded; }

    private:
        void setDefaultConfig();
        const ConfigItem &getConfigItem(ConfigKey key) const;
        std::string getConfigString(ConfigKey key) const;
        size_t getConfigSizeT(ConfigKey key) const;
        char getConfigChar(ConfigKey key) const;
        bool openfile(const std::string &filename, std::ofstream &file);
        void writeDefaultConfig();
        static bool writeDefaultConfig(std::ostream &file);
        std::string trim(const std::string &str) const;
    };
}
