#include "../include/buffer.hpp"

namespace Log
{
    Buffer::Buffer(const size_t buffsize)
        : _surplus_size(buffsize), _max_size(buffsize)
    {
    }

    void Buffer::push(const std::string &con)
    {
        _buffer += con;
        if (_surplus_size > con.size())
        {
            _surplus_size -= con.size();
        }
        else
        {
            _surplus_size = 0;
        }
    }

    void Buffer::swap(Buffer &buf)
    {
        std::swap(_surplus_size, buf._surplus_size);
        std::swap(_buffer, buf._buffer);
    }

    void Buffer::clear()
    {
        _buffer.clear();
        _surplus_size = _max_size;
    }
}
