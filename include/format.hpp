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
            // 时间输出样式
            TimeFormat(const std::string &tf)
            {
                if(tf.empty() || !istrue(tf)) {
                    _tf = Log::Data::defaultTF();
                }
                else
                {
                    _tf = tf;
                }
            }
            void format(std::ostream &out, const Log::Message &msg) override
            {
                out<< Data::GetFormatTime(msg._time,_tf.c_str());
            }  
        private:
            bool istrue(const std::string& tf)
            {
                for(size_t i = 0;i < tf.size();i++)
                {
                    if(tf[i] == '%' && !isop(tf[++i]))
                    {
                        return false;
                    }
                }
                return true;
            }
            bool isop(const char& op)
            {
                if(op == 'Y' || op == 'm' || op == 'd'\
                    || op == 'H' || op == 'M' || op == 'S') {return true;}
                else return false;
            }
        private:
            std::string _tf;
        };

        class LineFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override
            {
                out << std::to_string(msg._line);
            }
        };
        class LevelFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override
            {
                out << Log::LogLevel::toString(msg._value);
            }
        };
        class TidFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override
            {
                out << msg._tid;
            }
        };
        class FilenameFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override
            {
                out << msg._filename;
            }
        };
        class LoggerFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override
            {
                const std::string& name = msg._loggername;
                const std::string& tname = Log::Data::toString(msg._loggertype);
                if(tname == name)
                    out << name;
                else out << name << "_" << tname;
            }
        };
        class ContentFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override
            {
                out << msg._content;
            }
        };
        class NewlineFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override
            {
                out << "\n";
            }
        };
        class TabFormat : public FormatBase
        {
        public:
            void format(std::ostream &out, const Log::Message &msg) override
            {
                out << "\t";
            }
        };
        class OtherFormat : public FormatBase
        {
        public:
            OtherFormat(const std::string &other)
                : _other(other)
            {
            }
            void format(std::ostream &out, const Log::Message &msg) override
            {
                out << _other;
            }
        
        private:
            std::string _other;
        };

        
    }

    class Formatctrl : public Log::Format::FormatBase
        {
            typedef std::vector<std::shared_ptr<Format::FormatBase>> vfptr;
        public:
            Formatctrl(const std::string &format = Log::Data::defaultformat())
                : _format(format)
            {
                if(!formatana())
                {
                    _format = Log::Data::defaultformat();
                    _item.clear();
                    formatana();
                }
            }
            std::string format(const Log::Message &msg)
            {
                std::stringstream ss;
                format(ss, msg);
                return ss.str();
            }

        private:
            void format(std::ostream &out, const Log::Message &msg)
            {
                for (auto &item : _item)
                {
                    item->format(out, msg);
                }
            }
            Format::FormatBase::ptr createItem(char op,const std::string& str = "")
            {
                // 负责输出格式化样式控制
                //%L 日志等级
                //%N 日志名称
                //%Y %m %d %H %M %S %D时间相关
                //%f 文件名
                //%l 行号
                //%t 线程id
                //%c 文件内容
                //%n 换行
                //%T tab
                //%o 其他
                switch (op)
                {
                case 'L': return std::make_shared<Format::LevelFormat>();
                case 'N': return std::make_shared<Format::LoggerFormat>();
                case 'D': return std::make_shared<Format::TimeFormat>(str);
                case 'f': return std::make_shared<Format::FilenameFormat>();
                case 'l': return std::make_shared<Format::LineFormat>();
                case 't': return std::make_shared<Format::TidFormat>();
                case 'c': return std::make_shared<Format::ContentFormat>();
                case 'n': return std::make_shared<Format::NewlineFormat>();
                case 'T': return std::make_shared<Format::TabFormat>();
                default:  return std::make_shared<Format::OtherFormat>(str);
                }
            }
            bool formatana()
            {
                // "[%L][%N][{%Y-%m-%d %H:%M:%S}][%f][%l][%t][%c][%n]"
                size_t size = _format.size();
                for(size_t i = 0;i < size;i++)
                {
                    char op = _format[i];
                    if(op == '%')
                    {
                        char tmp = _format[++i];
                        if(isop(tmp))
                        {
                            _item.push_back(createItem((_format[i])));
                        }
                        else {return false;}
                    }
                    else if(op == '{')
                    {
                        auto pos = _format.find("}",i);
                        if(pos == std::string::npos) return false;
                        std::string str = _format.substr(i + 1,pos - i -1);
                        _item.push_back(createItem('D',str));
                        i = pos;
                    }
                    else
                    {
                        std::string s;
                        s+=_format[i];
                        _item.push_back(createItem('O',s));
                    }

                }
                return true;
            }
            bool isop(char op)
            {
                if(op == 'L' || op == 'N' || op == 'D' \
                    || op == 'f' || op == 'l' || op == 'c' \
                    || op == 'n' || op == 'T') {return true;}
                else {return false;}
            }
        private:
            std::string _format;
            vfptr _item;
        };
}
