#pragma once

#include "../assert.hpp"

namespace ek {

// TODO: move & copy constructor and assignment
template<typename T, unsigned MaxCount>
struct FixedArray {

    static_assert(MaxCount <= 0x10000);

    T _data[MaxCount];
    unsigned _size = 0;

    inline void push_back(T el) {
        EK_ASSERT(_size < MaxCount);
        _data[_size++] = el;
    }

    inline T* begin() {
        return _data;
    }

    inline T* end() {
        return _data + _size;
    }

    [[nodiscard]]
    inline unsigned size() const {
        return _size;
    }

    [[nodiscard]]
    inline const T* data() const {
        return _data;
    }

    [[nodiscard]]
    inline T* data() {
        return _data;
    }

    [[nodiscard]]
    inline T back() const {
        EK_ASSERT(_size > 0);
        return *(_data + _size - 1);
    }

    inline T& operator[](unsigned i) {
        EK_ASSERT(i < _size);
        return *(_data + i);
    }

    inline const T& operator[](unsigned i) const {
        EK_ASSERT(i < _size);
        return *(_data + i);
    }

    inline void clear() {
        _size = 0;
    }
};

}
