#pragma once
#include "logdata.hpp"
#include "tool.hpp"
#include <memory>
#include <fstream>
#include <atomic>
/*
    日志落地模块：
    1.解决日志的输出方向
    2.采用工厂模式，保证代码的可扩展性
*/
namespace Log
{
    class Sink
    {
    public:
        Sink()
        {
        }
        typedef std::shared_ptr<Sink> ptr;
        virtual ~Sink()
        {
        }
        virtual void WriteFile(const std::string &) = 0;
    };
    namespace SinkWay
    {
        class StdoutSink : public Sink
        {
        public:
            void WriteFile(const std::string &str) override
            {
                std::cout << str;
            }
        };
        class FiletSink : public Sink
        {
        public:
            FiletSink(const std::string &filepath)
                : _filepath(filepath)
            {
                tool::File::createFilePath(tool::File::GetFilepath(filepath));
                _ofs.open(_filepath, std::ofstream::ate | std::ofstream::app);
                if (!_ofs.good())
                {
                    std::cout << "FiletSink 文件打开失败,切换为默认文件" << std::endl;
                    _filepath = Data::defaultBFile();
                    _ofs.open(_filepath, std::ofstream::ate | std::ofstream::app);
                    if (!_ofs.good())
                    {
                        std::cout << "切换为默认文件失败" << std::endl;
                    }
                }
            }
            void WriteFile(const std::string &str) override
            {
                _ofs.write(str.c_str(), str.size());
            }

        private:
            std::string _filepath;
            std::ofstream _ofs;
        };

        class RollFileSink : public Sink
        {
        public:
            RollFileSink(size_t maxsize = Data::max_logfile_size(), const std::string &basefile = Data::defaultBFile())
                : _size(0), _num(1), _maxsize(maxsize), _basefile(basefile)
            {
                if (_basefile.empty() || _basefile == Data::defaultBFile())
                {
                    // 搜索默认日志目录下的最新文件进行写入
                    Init(); // 初始化查找最新文件
                }
                if (_filepath.empty())
                {
                    tool::File::createFilePath(tool::File::GetFilepath(_basefile));
                    openNewFile();
                }
                else
                {
                    // 检查最新文件大小是否超过默认大小
                    if (_size >= _maxsize)
                    {
                        // 如果超过则创建新文件
                        openNewFile();
                    }
                    else
                    {
                        // 否则直接打开最新文件
                        std::unique_lock<std::mutex> lock(_mutex);
                        _ofs.open(_filepath, std::ofstream::ate | std::ofstream::app);
                        if (!_ofs.good())
                        {
                            std::cout << "RollFileSink 文件打开失败，创建新文件" << std::endl;
                            openNewFile();
                        }
                    }
                }
            }
            ~RollFileSink() override
            {
            }
            void WriteFile(const std::string &str) override
            {

                _size += str.size();
                if (_size < _maxsize)
                {
                    _ofs.write(str.c_str(), str.size());
                }
                else
                {
                    Write(str);
                }
            }

        private:
            void Init()
            {
                // 默认路径下查找最新的日志文件
                _num = 1;
                _size = 0;
                _filepath.clear();

                // 获取默认基础文件名和目录
                std::string baseFile = Data::defaultBFile();
                std::string baseDir = tool::File::GetFilepath(baseFile);
                std::string baseName = baseFile.substr(baseDir.size());

                // 调用跨平台的文件查找方法
                std::string latestFile = tool::File::FindLatestLogFile(baseDir, baseName, Data::defaultFix(), _num, _size);

                // 如果找到最新文件，设置文件路径
                if (!latestFile.empty())
                {
                    _filepath = latestFile;
                    // 将找到的最新文件编号作为当前编号
                    // _num已经被FindLatestLogFile()设置为最新的文件编号
                }
            }

            void Write(const std::string &str)
            {

                size_t size = _size - _maxsize;
                if (size < Data::Exceed_size())
                {
                    //将包含超过部分写入当前文件
                    _ofs.write(str.c_str(), str.size());
                    openNewFile();
                }
                else
                {
                    
                    size_t len = str.size() - size;
                    _ofs.write(str.c_str(), len);
                    //将超过部分写入新文件
                    openNewFile();
                    std::string s = str.substr(len);
                    WriteFile(s);
                }
            }

            void openNewFile()
            {

                if (_ofs.good())
                {
                    _ofs.close();
                }
                _size = 0;
                createFilepath();
                std::unique_lock<std::mutex> lock(_mutex);
                _ofs.open(_filepath, std::ofstream::ate | std::ofstream::app);
                if (!_ofs.good())
                {
                    std::cout << "RollFileSink 文件打开失败" << std::endl;
                    _basefile = Data::defaultBFile();
                    openNewFile();
                }
            }
            void createFilepath()
            {

                std::string str = Data::GetFormatTime(tool::Date::GetTime(), Data::defaultFileTF());
                if (!str.empty())
                {
                    _num++;
                    _filepath = _basefile + std::to_string(_num) + Data::BoundSymbol() + str + Data::defaultFix();
                    if (_num >= Data::MaxFileSerial())
                        _num = 0;
                }
                else
                {
                    std::cout << "str为空" << std::endl;
                    // 错误处理
                }
            }

        private:
            std::atomic<size_t> _maxsize;
            std::atomic<size_t> _size;
            std::atomic<size_t> _num;
            std::string _basefile;
            std::string _filepath;
            std::ofstream _ofs;
            std::mutex _mutex;
        };

    }

    class SinkFactory
    {
    public:
        template <class SW, class... Args>
        static Sink::ptr SinkWay(Args &&...args)
        {
            return std::make_shared<SW>(std::forward<Args>(args)...);
        }

        static Sink::ptr StdoutSink()
        {
            return std::make_shared<SinkWay::StdoutSink>();
        }
        static Sink::ptr FiletSink(const std::string &filepath)
        {
            return std::make_shared<SinkWay::FiletSink>(filepath);
        }
        static Sink::ptr RollFileSink(size_t maxsize = Data::max_logfile_size(), const std::string &basefile = Data::defaultBFile())
        {
            return std::make_shared<SinkWay::RollFileSink>(maxsize, basefile);
        }
    };
}