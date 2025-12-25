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
            static const time_t GetTime()
            {
                return time(nullptr);
            }
           
        };

        class File
        {

        public:
            // 查看文件目录或普通文件是否存在
            static bool FileisExist(const std::string &filename)
            {
                struct stat buffer;
                return !stat(filename.c_str(), &buffer);
            }

            static const std::string GetFilepath(const std::string &filename)
            {
                //./abc/a.txt
                auto pos = filename.find_last_of("/\\");
                if (pos == std::string::npos)
                {
                    return ".";
                }
                else
                {
                    return filename.substr(0, pos + 1);
                }
            }

            static void createFilePath(std::string filename, mode_t mode = 0777)
            {
                // "./a/b/c/d/tmp.txt"
                int pos = 0, ori = 0;
                std::string tmp;
                while (true)
                {
                    pos = filename.find_first_of("/\\", ori);
                    if (pos == std::string::npos)
                    {
                        if (!FileisExist(filename))
                            mkdir(filename.c_str(), mode);
                        return;
                    }
                    tmp = filename.substr(0, pos);
                    if (!FileisExist(tmp))
                        mkdir(tmp.c_str(), mode);
                    ori = pos + 1;
                }
            }

            // 查找指定目录下的最新日志文件
            // baseDir: 日志文件所在目录
            // baseName: 日志文件基础名称
            // extension: 日志文件扩展名
            // maxNum: 输出参数，返回最大文件编号
            // fileSize: 输出参数，返回最新文件大小
            // 返回值: 最新日志文件的完整路径，若未找到则返回空字符串
            static std::string FindLatestLogFile(const std::string &baseDir, const std::string &baseName, 
                                                const std::string &extension, std::atomic<size_t> &maxNum, std::atomic<size_t> &fileSize)
            {
                maxNum.store(1);
                fileSize.store(0);
                std::string latestFile;
                // 修复查找模式，匹配 baseName + 数字 + "_*" 的格式
                std::string pattern = baseDir + baseName + "[0-9]*_*" + extension;

                // 创建目录（如果不存在）
                createFilePath(baseDir);

                // 检查目录是否存在
                if (!FileisExist(baseDir))
                {
                    return latestFile;
                }

#ifdef _WIN32
                // Windows平台实现
                WIN32_FIND_DATAA findData;
                HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);
                
                if (hFind != INVALID_HANDLE_VALUE)
                {
                    do
                    {
                        // 跳过目录
                        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        {
                            continue;
                        }

                        std::string filename = baseDir + findData.cFileName;
                        std::string filenameOnly = findData.cFileName;
                        
                        // 解析文件名获取编号
                        // 格式: baseName + 数字 + "_" + 时间 + "." + 扩展名
                        // 例如: "../logs/log1_20251221194639.txt"
                        if (filenameOnly.find(baseName) == 0)
                        {
                            // 找到基础文件名位置
                            size_t numStart = baseName.size();
                            size_t numEnd = filenameOnly.find("_", numStart);
                            if (numEnd != std::string::npos)
                            {
                                std::string numStr = filenameOnly.substr(numStart, numEnd - numStart);
                                try
                                {
                                    size_t fileNum = std::stoull(numStr);
                                    if (fileNum > maxNum) {
                                        maxNum.store(fileNum);
                                        latestFile = filename;
                                    }
                                    else if (fileNum == maxNum)
                                    {
                                        // 当文件编号相同时，比较时间戳，选择最新的文件
                                        // 时间戳在文件名的_之后，扩展名之前
                                        size_t timeStart = numEnd + 1;
                                        size_t timeEnd = filenameOnly.find_last_of(".");
                                        if (timeEnd > timeStart)
                                        {
                                            std::string currentTimeStr = latestFile.empty() ? "" : 
                                                latestFile.substr(latestFile.find_last_of("/") + 1);
                                            size_t currentTimeStart = currentTimeStr.find_last_of("_") + 1;
                                            size_t currentTimeEnd = currentTimeStr.find_last_of(".");
                                            if (currentTimeEnd > currentTimeStart)
                                            {
                                                std::string currentTime = currentTimeStr.substr(currentTimeStart, currentTimeEnd - currentTimeStart);
                                                std::string newTime = filenameOnly.substr(timeStart, timeEnd - timeStart);
                                                if (newTime > currentTime)
                                                {
                                                    latestFile = filename;
                                                }
                                            }
                                            else
                                            {
                                                // 如果当前文件的时间戳解析失败，使用新文件
                                                latestFile = filename;
                                            }
                                        }
                                    }
                                }
                                catch (...)
                                {
                                    // 忽略解析错误
                                }
                            }
                        }
                    } while (FindNextFileA(hFind, &findData));
                    
                    FindClose(hFind);
                }
#else
                // Linux平台实现
                glob_t glob_result;
                
                if (glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result) == 0)
                {
                    for (size_t i = 0; i < glob_result.gl_pathc; ++i)
                    {
                        std::string filename = glob_result.gl_pathv[i];
                        
                        // 解析文件名获取编号
                        // 格式: baseName + 数字 + "_" + 时间 + "." + 扩展名
                        // 例如: "../logs/log1_20251221194639.txt"
                        
                        // 获取路径中的文件名部分
                        size_t pos = filename.find_last_of("/\\");
                        std::string filenameOnly = (pos != std::string::npos) ? filename.substr(pos + 1) : filename;
                        
                        // 检查文件名是否以baseName开头
                        if (filenameOnly.find(baseName) == 0)
                        {
                            // 找到基础文件名位置
                            size_t numStart = baseName.size();
                            size_t numEnd = filenameOnly.find("_", numStart);
                            if (numEnd != std::string::npos)
                            {
                                std::string numStr = filenameOnly.substr(numStart, numEnd - numStart);
                                try
                                {
                                    size_t fileNum = std::stoull(numStr);
                                    if (fileNum > maxNum) {
                                        maxNum.store(fileNum);
                                        latestFile = filename;
                                    }
                                    else if (fileNum == maxNum)
                                    {
                                        // 当文件编号相同时，比较时间戳，选择最新的文件
                                        // 时间戳在文件名的_之后，扩展名之前
                                        size_t timeStart = numEnd + 1;
                                        size_t timeEnd = filenameOnly.find_last_of(".");
                                        if (timeEnd > timeStart)
                                        {
                                            std::string currentTimeStr = latestFile.empty() ? "" : 
                                                latestFile.substr(latestFile.find_last_of("/") + 1);
                                            size_t currentTimeStart = currentTimeStr.find_last_of("_") + 1;
                                            size_t currentTimeEnd = currentTimeStr.find_last_of(".");
                                            if (currentTimeEnd > currentTimeStart)
                                            {
                                                std::string currentTime = currentTimeStr.substr(currentTimeStart, currentTimeEnd - currentTimeStart);
                                                std::string newTime = filenameOnly.substr(timeStart, timeEnd - timeStart);
                                                if (newTime > currentTime)
                                                {
                                                    latestFile = filename;
                                                }
                                            }
                                            else
                                            {
                                                // 如果当前文件的时间戳解析失败，使用新文件
                                                latestFile = filename;
                                            }
                                        }
                                    }
                                }
                                catch (...)
                                {
                                    // 忽略解析错误
                                }
                            }
                        }
                    }
                    
                    globfree(&glob_result);
                }
#endif

                // 如果找到最新文件，获取文件大小
                if (!latestFile.empty())
                {
                    struct stat st;
                    if (stat(latestFile.c_str(), &st) == 0)
                    {
                        fileSize.store(static_cast<size_t>(st.st_size));
                    }
                }

                return latestFile;
            }
        };

    }

}
