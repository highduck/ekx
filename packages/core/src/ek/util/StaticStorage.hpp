#pragma once

#include <ek/assert.h>

namespace ek {

template<typename T>
class alignas(alignof(T)) StaticStorage {
    char buffer[sizeof(T)];

    union Converter {
        const void* data;
        T* ptr;
    };

public:

    template<typename ...Args>
    void initialize(Args&& ...args) {
        new(buffer) T(args...);
    }

    constexpr T& get() const {
        Converter u{buffer};
        return *u.ptr;
    }

    constexpr T* ptr() const {
        Converter u{buffer};
        return u.ptr;
    }

    constexpr T& ref() const {
        Converter u{buffer};
        return *u.ptr;
    }

    void shutdown() {
        Converter{buffer}.ptr->~T();
    }
};

}
