#pragma once
#include <iostream>
#include "ConfigManager.hpp"
/*
    用于日志文件信息的暂时缓存,通过析构写入配置文件
*/

namespace Log
{
    class logmessage
    {
    public:
        static logmessage& getInstance()
        {
            static logmessage ince;
            return ince;
        }
        void addlogmessage(size_t num,size_t size,const std::string& filename)
        {
            if(size >= _size)
            {
                _size = size;
                _num = num;
                _filename = filename;
            }
        }

    private:
        logmessage()
            :_size(0),_num(1)
        {
        }
        ~logmessage()
        {
            Log::ConfigManager::getInstance().writeLogFileConfig(_num,_size,_filename);
        }
        // 日志编号,大小,名称
        size_t _num;
        size_t _size;
        std::string _filename;
    };

}
