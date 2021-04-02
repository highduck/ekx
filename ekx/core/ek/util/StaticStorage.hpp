#pragma once

#include "../assert.hpp"

namespace ek {

template<typename T>
class alignas(alignof(T)) StaticStorage {
    union {
        T value;
        char buffer[sizeof(T) + 1];
    };
public:
    inline constexpr StaticStorage() noexcept {
        buffer[sizeof(T)] = 0;
    }

    inline ~StaticStorage() noexcept {
        // we are exit the process
        //EK_ASSERT(!isInitialized());
    }

    [[nodiscard]]
    inline bool isInitialized() const {
        return buffer[sizeof(T)] != 0;
    }

    template<typename ...Args>
    inline void initialize(Args&& ...args) {
        EK_ASSERT(!isInitialized());
        buffer[sizeof(T)] = 1;
        new(buffer) T(args...);
    }

    constexpr inline T& get() {
        EK_ASSERT(isInitialized());
        return value;
    }

    constexpr inline T* ptr() {
        return (T*) &value;
    }

    constexpr inline T& ref() {
        return value;
    }

    inline void shutdown() {
        EK_ASSERT(isInitialized());
        value.~T();
        buffer[sizeof(T)] = 0;
    }
};

}
