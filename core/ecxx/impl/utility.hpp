#pragma once

#ifndef NDEBUG

#include <cassert>

#define ECXX_ASSERT(x) assert(x)
#define ECXX_FULL_ASSERT(x) assert(x)

#else

#define ECXX_ASSERT(ignore) ((void)0)
#define ECXX_FULL_ASSERT(ignore) ((void)0)

#endif

namespace ecs::details {

struct identity_counter {
    static unsigned counter;

    inline static unsigned next() {
        return counter++;
    }
};

inline unsigned identity_counter::counter{0};

// 1. match between TU
// 2. starts from 0 for each Identity type
template<typename T>
struct identity_generator {
    static const unsigned value;
};

template<typename T>
inline const unsigned identity_generator<T>::value = details::identity_counter::next();

}