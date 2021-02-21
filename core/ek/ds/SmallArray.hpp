#pragma once

#include "../assert.hpp"

namespace ek {

template<typename T, unsigned MaxCount>
struct SmallArray {

    static_assert(MaxCount <= 0x10000);

    unsigned size = 0;
    T data[MaxCount];

    inline void push(T el) {
        EK_ASSERT_R2(size < MaxCount);
        data[size++] = el;
    }
};

}
