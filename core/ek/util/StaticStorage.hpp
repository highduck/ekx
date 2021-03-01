#pragma once

#include "../assert.hpp"

namespace ek {

template<typename T>
class StaticStorage {
    bool initialized = false;
    char buffer[sizeof(T)];
public:
    template<typename ...Args>
    inline void initialize(Args&& ...args) {
        EK_ASSERT(!initialized);
        initialized = true;

        new(buffer) T(args...);
    }

    constexpr inline T& get() {
        EK_ASSERT(initialized);
        return *((T*) buffer);
    }

    constexpr inline T* ptr() {
        return (T*) buffer;
    }

    inline void shutdown() {
        EK_ASSERT(initialized);
        ((T*) buffer)->~T();
        memory::clear(buffer, sizeof(T));
        initialized = false;
    }
};

}
