#pragma once
#include "message.hpp"
#include "level.hpp"
#include "logdata.hpp"
#include <memory>
#include <vector>
#include <ostream>
#include <sstream>
// 日志消息格式化模块
namespace Log
{
    namespace Format
    {
        class FormatBase
        {
        public:
            typedef std::shared_ptr<FormatBase> ptr;
            virtual void format(std::ostream &out, const Log::Message &msg) = 0;
        };

        class TimeFormat : public FormatBase
        {
        public:
            TimeFormat(const std::string &tf);
            void format(std::ostream &out, const Log::Message &msg) override;

        private:
            bool istrue(const std::string &tf);
            bool isop(const char &op);

        private:
            std::string _tf;
        };

        class LineFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override;
        };
        class LevelFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override;
        };
        class TidFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override;
        };
        class FilenameFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override;
        };
        class LoggerFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override;
        };
        class ContentFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override;
        };
        class NewlineFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override;
        };
        class TabFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override;
        };
        class OtherFormat : public FormatBase
        {
        public:
            OtherFormat(const std::string &other);
            void format(std::ostream &out, const Log::Message &msg) override;

        private:
            std::string _other;
        };

        
    }

    class Formatctrl : public Log::Format::FormatBase
        {
            typedef std::vector<std::shared_ptr<Format::FormatBase>> vfptr;
        public:
            Formatctrl(const std::string &format = Log::Data::defaultformat());
            std::string format(const Log::Message &msg);

        private:
            void format(std::ostream &out, const Log::Message &msg);
            Format::FormatBase::ptr createItem(char op,const std::string& str = "");
            bool formatana();
            bool isop(char op);
        private:
            std::string _format;
            vfptr _item;
        };
}
