#pragma once

#include "../Allocator.hpp"

namespace ek {

template<typename T>
struct DynArray {

    unsigned capacity;
    unsigned size;
    T* data;

    DynArray() {
        size = 0;
        capacity = 64;
        data = (T*) EK_MALLOC(sizeof(T) * capacity);
    }

    ~DynArray() {
        EK_FREE(data);
    }

    [[nodiscard]]
    inline bool empty() const {
        return size == 0;
    }

    inline void clear() {
        size = 0;
    }

    void grow() {
        capacity = capacity << 1;
        data = (T*) EK_REALLOC(data, sizeof(T) * capacity);
    }

    inline void push_back(T el) {
        if (size == capacity) {
            grow();
        }
        *(data + size) = el;
        ++size;
    }

    inline void set(unsigned i, T el) const {
        EK_ASSERT_R2(i < size);
        *(data + i) = el;
    }

    [[nodiscard]]
    inline T get(unsigned i) const {
        EK_ASSERT_R2(i < size);
        return *(data + i);
    }

    inline void remove_back() {
        --size;
    }

    [[nodiscard]]
    inline T back() const {
        return *(data + size - 1);
    }
};

}