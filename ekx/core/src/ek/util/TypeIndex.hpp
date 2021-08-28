#pragma once

#include <cstdint>

namespace ek {

template<typename Kind = void>
inline uint32_t nextTypeIndex() {
    static uint32_t g = 0;
    return g++;
}

template<typename T, typename Kind = void>
inline uint32_t typeIndex() {
    static uint32_t index = nextTypeIndex<Kind>();
    return index;
}

template<typename T, typename Kind = void>
struct TypeIndex {
    static const uint32_t value;
};

template<typename T, typename Kind>
inline const uint32_t TypeIndex<T, Kind>::value = typeIndex<T, Kind>();

}