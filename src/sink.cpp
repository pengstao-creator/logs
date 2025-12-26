#include "../include/sink.hpp"
#include <iostream>

namespace Log
{
    void SinkWay::StdoutSink::WriteFile(const std::string &str)
    {
        std::cout << str;
    }

    SinkWay::FiletSink::FiletSink(const std::string &filepath)
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

    void SinkWay::FiletSink::WriteFile(const std::string &str)
    {
        _ofs.write(str.c_str(), str.size());
    }

    SinkWay::RollFileSink::RollFileSink(size_t maxsize, const std::string &basefile)
        : _size(0), _num(1), _maxsize(maxsize), _basefile(basefile)
    {
        if (_basefile.empty() || _basefile == Data::defaultBFile())
        {
            Init();
        }
        if (_filepath.empty())
        {
            tool::File::createFilePath(tool::File::GetFilepath(_basefile));
            openNewFile();
        }
        else
        {
            if (_size >= _maxsize)
            {
                openNewFile();
            }
            else
            {
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

    SinkWay::RollFileSink::~RollFileSink()
    {
    }

    void SinkWay::RollFileSink::WriteFile(const std::string &str)
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

    void SinkWay::RollFileSink::Init()
    {
        _num = 1;
        _size = 0;
        _filepath.clear();

        std::string baseFile = Data::defaultBFile();
        std::string baseDir = tool::File::GetFilepath(baseFile);
        std::string baseName = baseFile.substr(baseDir.size());

        std::string latestFile = tool::File::FindLatestLogFile(baseDir, baseName, Data::defaultFix(), _num, _size);

        if (!latestFile.empty())
        {
            _filepath = latestFile;
        }
    }

    void SinkWay::RollFileSink::Write(const std::string &str)
    {
        size_t size = _size - _maxsize;
        if (size < Data::Exceed_size())
        {
            _ofs.write(str.c_str(), str.size());
            openNewFile();
        }
        else
        {
            size_t len = str.size() - size;
            _ofs.write(str.c_str(), len);
            openNewFile();
            std::string s = str.substr(len);
            WriteFile(s);
        }
    }

    void SinkWay::RollFileSink::openNewFile()
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

    void SinkWay::RollFileSink::createFilepath()
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
        }
    }
}
