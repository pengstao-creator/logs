#include "../include/pch.hpp"
#include "../include/format.hpp"

namespace Log
{
    Format::TimeFormat::TimeFormat(const std::string &tf)
        : _tf(tf)
    {
    }

    void Format::TimeFormat::format(std::ostream &out, const Log::Message &msg)
    {
        out << Data::GetFormatTime(msg._time, _tf.c_str());
    }

    bool Format::TimeFormat::istrue(const std::string &tf)
    {
        for (size_t i = 0; i < tf.size(); i++)
        {
            if (tf[i] == '%' && !isop(tf[++i]))
            {
                return false;
            }
        }
        return true;
    }

    bool Format::TimeFormat::isop(const char &op)
    {
        if (op == 'Y' || op == 'm' || op == 'd' ||
            op == 'H' || op == 'M' || op == 'S')
        {
            return true;
        }
        else
            return false;
    }

    void Format::LineFormat::format(std::ostream &out, const Log::Message &msg)
    {
        out << std::to_string(msg._line);
    }

    void Format::LevelFormat::format(std::ostream &out, const Log::Message &msg)
    {
        out << Log::LogLevel::toString(msg._value);
    }

    void Format::TidFormat::format(std::ostream &out, const Log::Message &msg)
    {
        out << msg._tid;
    }

    void Format::FilenameFormat::format(std::ostream &out, const Log::Message &msg)
    {
        out << msg._filename;
    }

    void Format::LoggerFormat::format(std::ostream &out, const Log::Message &msg)
    {
        const std::string &name = msg._loggername;
        const std::string &tname = Log::Data::toString(msg._loggertype);
        if (tname == name)
            out << name;
        else
            out << name << "_" << tname;
    }

    void Format::ContentFormat::format(std::ostream &out, const Log::Message &msg)
    {
        out << msg._content;
    }

    void Format::NewlineFormat::format(std::ostream &out, const Log::Message &msg)
    {
        out << "\n";
    }

    void Format::TabFormat::format(std::ostream &out, const Log::Message &msg)
    {
        out << "\t";
    }

    Format::OtherFormat::OtherFormat(const std::string &other)
        : _other(other)
    {
    }

    void Format::OtherFormat::format(std::ostream &out, const Log::Message &msg)
    {
        out << _other;
    }

    Formatctrl::Formatctrl(const std::string &format)
        : _format(format)
    {
        formatana();
    }

    std::string Formatctrl::format(const Log::Message &msg)
    {
        std::stringstream ss;
        format(ss, msg);
        return ss.str();
    }

    void Formatctrl::format(std::ostream &out, const Log::Message &msg)
    {
        for (auto &item : _item)
        {
            item->format(out, msg);
        }
    }

    Format::FormatBase::ptr Formatctrl::createItem(char op, const std::string &str)
    {
        switch (op)
        {
        case 'L':
            return std::make_shared<Format::LevelFormat>();
        case 'N':
            return std::make_shared<Format::LoggerFormat>();
        case 'D':
            return std::make_shared<Format::TimeFormat>(str);
        case 'f':
            return std::make_shared<Format::FilenameFormat>();
        case 'l':
            return std::make_shared<Format::LineFormat>();
        case 't':
            return std::make_shared<Format::TidFormat>();
        case 'c':
            return std::make_shared<Format::ContentFormat>();
        case 'n':
            return std::make_shared<Format::NewlineFormat>();
        case 'T':
            return std::make_shared<Format::TabFormat>();
        default:
            return std::make_shared<Format::OtherFormat>(str);
        }
    }

    bool Formatctrl::formatana()
    {
        size_t size = _format.size();
        for (size_t i = 0; i < size; i++)
        {
            char op = _format[i];
            if (op == '%')
            {
                char tmp = _format[++i];
                if (isop(tmp))
                {
                    _item.push_back(createItem((_format[i])));
                }
                else
                {
                    return false;
                }
            }
            else if (op == '{')
            {
                auto pos = _format.find("}", i);
                if (pos == std::string::npos)
                    return false;
                std::string str = _format.substr(i + 1, pos - i - 1);
                _item.push_back(createItem('D', str));
                i = pos;
            }
            else
            {
                std::string s;
                s += _format[i];
                _item.push_back(createItem('O', s));
            }
        }
        return true;
    }

    bool Formatctrl::isop(char op)
    {
        if (op == 'L' || op == 'N' || op == 'D' ||
            op == 'f' || op == 'l' || op == 'c' ||
            op == 'n' || op == 'T')
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}
