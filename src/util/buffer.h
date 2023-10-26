#pragma once

#include <cstddef>

template <typename T>
class Buffer
{
private: 
    T* ptr;
    size_t count;
public:
    Buffer(T* ptr, size_t size)
    {
        this->ptr = ptr;
        this->count = size;
    }

    T* data() const { return ptr; }
    size_t size() const { return count; }
    T& operator[](size_t index) { return ptr[index]; }
};
