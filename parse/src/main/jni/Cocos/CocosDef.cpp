//
// Created by Apple on 2019/5/7.
//

#include <cstring>
#include "CocosDef.h"

const Data Data::Null;

Data::Data() :
        _bytes(nullptr),
        _size(0)
{
    //CCLOGINFO("In the empty constructor of Data.");
}

Data::Data(Data&& other) :
        _bytes(nullptr),
        _size(0)
{
    //CCLOGINFO("In the move constructor of Data.");
    move(other);
}

Data::Data(const Data& other) :
        _bytes(nullptr),
        _size(0)
{
    //CCLOGINFO("In the copy constructor of Data.");
    copy(other._bytes, other._size);
}

Data::~Data()
{
    //CCLOGINFO("deallocing Data: %p", this);
    clear();
}

Data& Data::operator= (const Data& other)
{
    //CCLOGINFO("In the copy assignment of Data.");
    copy(other._bytes, other._size);
    return *this;
}

Data& Data::operator= (Data&& other)
{
    //CCLOGINFO("In the move assignment of Data.");
    move(other);
    return *this;
}

void Data::move(Data& other)
{
    clear();

    _bytes = other._bytes;
    _size = other._size;

    other._bytes = nullptr;
    other._size = 0;
}

bool Data::isNull() const
{
    return (_bytes == nullptr || _size == 0);
}

unsigned char* Data::getBytes() const
{
    return _bytes;
}

ssize_t Data::getSize() const
{
    return _size;
}

void Data::copy(const unsigned char* bytes, const ssize_t size)
{
    clear();

    if (size > 0)
    {
        _size = size;
        _bytes = (unsigned char*)malloc(sizeof(unsigned char) * _size);
        memcpy(_bytes, bytes, _size);
    }
}

void Data::fastSet(unsigned char* bytes, const ssize_t size)
{
    _bytes = bytes;
    _size = size;
}

void Data::clear()
{
    free(_bytes);
    _bytes = nullptr;
    _size = 0;
}

unsigned char* Data::takeBuffer(ssize_t* size)
{
    auto buffer = getBytes();
    if (size)
        *size = getSize();
    fastSet(nullptr, 0);
    return buffer;
}