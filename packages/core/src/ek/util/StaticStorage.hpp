#pragma once

#include <ek/assert.h>

namespace ek {

template<typename T>
class alignas(alignof(T)) StaticStorage {
    char buffer[sizeof(T) + 1];

    union Converter {
        const void* data;
        T* ptr;
    };

public:

//    constexpr StaticStorage() noexcept {
//        buffer[sizeof(T)] = 0;
//    }

//    ~StaticStorage() noexcept {
//         we are exit the process
//        EK_ASSERT(!isInitialized());
//    }

    [[nodiscard]]
    bool isInitialized() const {
        return buffer[sizeof(T)] != 0;
    }

    template<typename ...Args>
    void initialize(Args&& ...args) {
        EK_ASSERT(!isInitialized());
        buffer[sizeof(T)] = 1;
        new(buffer) T(args...);
    }

    constexpr T& get() const {
        EK_ASSERT(isInitialized());
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
        EK_ASSERT(isInitialized());
        Converter{buffer}.ptr->~T();
        buffer[sizeof(T)] = 0;
    }
};

}
