#include "../include/tool.hpp"

namespace Log
{
    namespace tool
    {
        const time_t Date::GetTime()
        {
            return time(nullptr);
        }

        bool File::FileisExist(const std::string &filename)
        {
            struct stat buffer;
            return !stat(filename.c_str(), &buffer);
        }

        const std::string File::GetFilepath(const std::string &filename)
        {
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

        void File::createFilePath(std::string filename, mode_t mode)
        {
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

        std::string File::FindLatestLogFile(const std::string &baseDir, const std::string &baseName, 
                                            const std::string &extension, std::atomic<size_t> &maxNum, std::atomic<size_t> &fileSize)
        {
            maxNum.store(1);
            fileSize.store(0);
            std::string latestFile;
            std::string pattern = baseDir + baseName + "[0-9]*_*" + extension;

            createFilePath(baseDir);

            if (!FileisExist(baseDir))
            {
                return latestFile;
            }

#ifdef _WIN32
            WIN32_FIND_DATAA findData;
            HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);
            
            if (hFind != INVALID_HANDLE_VALUE)
            {
                do
                {
                    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        continue;
                    }

                    std::string filename = baseDir + findData.cFileName;
                    std::string filenameOnly = findData.cFileName;
                    
                    if (filenameOnly.find(baseName) == 0)
                    {
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
                                            latestFile = filename;
                                        }
                                    }
                                }
                            }
                            catch (...)
                            {
                            }
                        }
                    }
                } while (FindNextFileA(hFind, &findData));
                
                FindClose(hFind);
            }
#else
            glob_t glob_result;
            
            if (glob(pattern.c_str(), GLOB_TILDE, nullptr, &glob_result) == 0)
            {
                for (size_t i = 0; i < glob_result.gl_pathc; ++i)
                {
                    std::string filename = glob_result.gl_pathv[i];
                    
                    size_t pos = filename.find_last_of("/\\");
                    std::string filenameOnly = (pos != std::string::npos) ? filename.substr(pos + 1) : filename;
                    
                    if (filenameOnly.find(baseName) == 0)
                    {
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
                                            latestFile = filename;
                                        }
                                    }
                                }
                            }
                            catch (...)
                            {
                            }
                        }
                    }
                }
                
                globfree(&glob_result);
            }
#endif

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

    }

}
