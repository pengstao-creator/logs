#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include <cstddef>
#include <cstdio>
#include <atomic>
// 跨平台头文件和宏定义
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define stat _stat
#define mkdir _mkdir
#else
#include <sys/stat.h>
#include <unistd.h>
#include <glob.h>
#endif
//常用工具
//1.获取系统时间
//2.判断文件是否存在
//3.获取文件路径
//4.创建文件
//

namespace Log
{
    namespace tool
    {
        class Date
        {
        public:
            static const time_t GetTime();
        };

        class File
        {

        public:
            static bool FileisExist(const std::string &filename);
            static const std::string GetFilepath(const std::string &filename);
            static void createFilePath(std::string filename, mode_t mode = 0777);
            static std::string FindLatestLogFile(const std::string &baseDir, const std::string &baseName, 
                                                const std::string &extension, std::atomic<size_t> &maxNum, std::atomic<size_t> &fileSize);
        };

    }

}
