#pragma once
#include "asserts.h"

// this template isnt used very much in v95

template <typename T, typename U>
class ZPair
{
private:
    T first;
    U second;

public:

    ZPair() = default;

    ZPair(T f, U s)
    {
        this->first = f;
        this->second = s;
    }

    ~ZPair()
    {
        delete first;
        delete second;
    }

    void SetFirst(T const item)
    {
        this->first = item;
    }

    void SetSecond(U const item)
    {
        this->second = item;
    }

    T* GetFirst() const
    {
        return &this->first;
    }

    U* GetSecond() const
    {
        return &this->second;
    }
};

assert_size(sizeof(ZPair<long, long>), 0x08);