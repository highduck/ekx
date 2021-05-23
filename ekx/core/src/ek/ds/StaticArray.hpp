#pragma once

#include "../assert.hpp"

namespace ek {

template<typename T, unsigned N>
struct StaticArray {

    static_assert(N <= 0x10000);
    static_assert(N > 0);

    T _data[N];

    inline T* begin() {
        return _data;
    }

    inline T* end() {
        return _data + N;
    }

    [[nodiscard]]
    inline unsigned size() const {
        return N;
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
        EK_ASSERT(N > 0);
        return *(_data + N - 1);
    }

    inline T& operator[](unsigned i) {
        EK_ASSERT(i < N);
        return *(_data + i);
    }

    inline const T& operator[](unsigned i) const {
        EK_ASSERT(i < N);
        return *(_data + i);
    }
};

}
