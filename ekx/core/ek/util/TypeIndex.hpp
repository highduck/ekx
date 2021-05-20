#pragma once

#include <cstdint>

namespace ek {
namespace details {

template<typename Kind>
struct TypeIndexCounter {
    static uint32_t value;

    inline static uint32_t next() {
        return value++;
    }
};

template<typename Kind>
uint32_t TypeIndexCounter<Kind>::value{0};

}

// 1. match between TU
// 2. starts from 0 for each Kind types family
template<typename T, typename Kind = void>
struct TypeIndex {
    static const uint32_t value;
};

template<typename T, typename Kind>
const uint32_t TypeIndex<T, Kind>::value = details::TypeIndexCounter<Kind>::next();

}