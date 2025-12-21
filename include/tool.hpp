#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
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

        class Flie
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
        };

    }

}
