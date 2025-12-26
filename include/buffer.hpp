#pragma once
#include "logdata.hpp"
#include <vector>
/*
    日志写入和读取的缓冲区
    在异步写入时，采用双缓冲区的方式进行写入
    一个线程负责写入缓冲区，一个线程负责写入磁盘
    当写入缓存区写完一次日志后会交换缓冲区
*/
namespace Log
{
    class Buffer
    {
    public:
        Buffer(const size_t buffsize = Data::max_buffer_size());
        void push(const std::string &con);
        void swap(Buffer &buf);
        bool empty() const { return _surplus_size == _max_size; }
        const std::string &ReadBuffer() const { return _buffer; }
        size_t WriteBSize() const { return _surplus_size; }
        void clear();

    private:
        size_t _surplus_size;
        size_t _max_size;
        std::string _buffer;
    };

}
