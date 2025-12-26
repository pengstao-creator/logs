#include "../include/pch.hpp"
#include "../include/ConfigManager.hpp"
#include <algorithm>

namespace Log
{
    bool ConfigManager::loadConfig(const std::string &filename)
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

    std::string ConfigManager::getString(const std::string &key, const std::string &defaultValue) const
    {
        auto it = _configMap.find(key);
        if (it != _configMap.end())
            return it->second;
        return defaultValue;
    }

    size_t ConfigManager::getSizeT(const std::string &key, size_t defaultValue) const
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

    char ConfigManager::getChar(const std::string &key, char defaultValue) const
    {
        auto it = _configMap.find(key);
        if (it != _configMap.end() && !it->second.empty())
        {
            return it->second[0];
        }
        return defaultValue;
    }

    bool ConfigManager::getBool(const std::string &key, bool defaultValue) const
    {
        auto it = _configMap.find(key);
        if (it != _configMap.end())
        {
            const std::string &value = it->second;
            return value == "true" || value == "1" || value == "TRUE" || value == "YES" || value == "yes";
        }
        return defaultValue;
    }

    const ConfigManager::ConfigItem &ConfigManager::getConfigItem(ConfigKey key) const
    {
        return configItems()[key];
    }

    std::string ConfigManager::getConfigString(ConfigKey key) const
    {
        const auto &item = getConfigItem(key);
        return getString(item.key, item.defaultValue);
    }

    size_t ConfigManager::getConfigSizeT(ConfigKey key) const
    {
        const auto &item = getConfigItem(key);
        return getSizeT(item.key, std::stoull(item.defaultValue));
    }

    char ConfigManager::getConfigChar(ConfigKey key) const
    {
        const auto &item = getConfigItem(key);
        return getChar(item.key, item.defaultValue[0]);
    }

    void ConfigManager::setDefaultConfig()
    {
        for (const auto &item : configItems())
        {
            _configMap[item.key] = item.defaultValue;
        }
    }

    bool ConfigManager::openfile(const std::string &filename, std::ofstream &file)
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

    void ConfigManager::writeDefaultConfig()
    {
        std::ofstream file;
        if (openfile(PropertiesName, file))
        {
            writeDefaultConfig(file);
            file.close();
        }
    }

    bool ConfigManager::writeDefaultConfig(std::ostream &file)
    {
        file << "# 日志配置文件\n\n";

        for (const auto &item : configItems())
        {
            if (item.key == "log.logFileName" || item.key == "log.file_serial" || item.key == "log.logFileSize")
            {
                continue;
            }

            file << "# " << item.description << "\n";
            file << item.key << "=" << getInstance()._configMap[item.key] << "\n\n";
        }

        return true;
    }

    std::string ConfigManager::trim(const std::string &str) const
    {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
            return "";

        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
}
