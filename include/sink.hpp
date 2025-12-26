#pragma once
#include "logdata.hpp"
#include "tool.hpp"
#include <memory>
#include <fstream>
#include <atomic>
#include <iostream>
#include <mutex>

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
            void WriteFile(const std::string &str) override;
        };

        class FiletSink : public Sink
        {
        public:
            FiletSink(const std::string &filepath);
            void WriteFile(const std::string &str) override;

        private:
            std::string _filepath;
            std::ofstream _ofs;
        };

        class RollFileSink : public Sink
        {
        public:
            RollFileSink(size_t maxsize = Data::max_logfile_size(), const std::string &basefile = Data::defaultBFile());
            ~RollFileSink() override;
            void WriteFile(const std::string &str) override;

        private:
            void Init();
            void Write(const std::string &str);
            void openNewFile();
            void createFilepath();

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
