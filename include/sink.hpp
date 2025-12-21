#pragma once
#include "logdata.hpp"
#include "tool.hpp"
#include "logmessage.hpp"
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
        typedef std::shared_ptr<Sink> ptr;
        virtual ~Sink()
        {
            if (num)
            {
                logmessage::getInstance().addlogmessage(num, size, filename);
            }
        }
        virtual void WriteFile(const std::string &) = 0;
        virtual bool isroll() = 0;
        // 记录终止时日志文件的状态用于写入配置文件
        std::atomic<size_t> num;
        size_t size;
        std::string filename;
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
            bool isroll() override
            {
                return false;
            }
        };
        class FiletSink : public Sink
        {
        public:
            FiletSink(const std::string &filepath)
                : _filepath(filepath)
            {
                tool::Flie::createFilePath(tool::Flie::GetFilepath(filepath));
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
            bool isroll() override
            {
                return false;
            }

        private:
            std::string _filepath;
            std::ofstream _ofs;
        };

        class RollFileSink : public Sink
        {
        public:
            RollFileSink(size_t maxsize = Data::max_logfile_size(), const std::string &basefile = Data::defaultBFile())
                : _pfile(false), _size(0), _num(1), _maxsize(maxsize), _basefile(basefile)
            {
                if (_basefile.empty() || _basefile == Data::defaultBFile())
                {
                    Init();
                    if (istruep() && tool::Flie::FileisExist(_filepath))
                    {
                        _pfile = true;
                        openNewFile();
                    }
                    else
                    {
                        _basefile = Data::defaultBFile();
                        _num = 1;
                    }
                }
                if (!_pfile)
                {
                    tool::Flie::createFilePath(tool::Flie::GetFilepath(_basefile));
                    openNewFile();
                }
            }
            ~RollFileSink() override
            {
                num = _num - 1;
                size = _size;
                filename = _filepath;
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
            bool isroll() override
            {
                return true;
            }
            const size_t getnum() const { return _num; }
            const size_t getsize() const { return _size; }
            const std::string &getlogfilename() const { return _filepath; }

        private:
            void Init()
            {
                // 从读取配置文件
                _size = Data::LogFileSize();
                _num = Data::LogFileSerial();
                _filepath = Data::LogFileName();
            }
            // 初步判断是否是正确配置
            bool istruep()
            {
                if ((_num <= Data::MaxFileSerial() && _num > 0) && (_size <= _maxsize) && !_filepath.empty())
                {
                    return true;
                }
                return false;
            }

            void Write(const std::string &str)
            {
                size_t size = _size - _maxsize;
                size_t len = str.size() - size;
                _ofs.write(str.c_str(), len);
                openNewFile();
                std::string s = str.substr(len);
                WriteFile(s);
            }

            void openNewFile()
            {

                if (_ofs.good())
                {
                    _ofs.close();
                }
                if (!_pfile || _size > _maxsize)
                {
                    _size = 0;
                    createFilepath();
                }
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
                    _filepath = _basefile + std::to_string(_num) + Data::BoundSymbol() + str + Data::defaultFix();
                    if (_num >= Data::MaxFileSerial())
                        _num = 0;
                    _num++;
                }
                else
                {
                    std::cout << "str为空" << std::endl;
                    // 错误处理
                }
            }

        private:
            std::atomic<bool> _pfile; // 打开配文件中的文件
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