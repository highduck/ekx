#pragma once

#ifndef NDEBUG
#define ECXX_ENABLE_ASSERT
#endif

// just use for full debug
//#define ECXX_ENABLE_ASSERT_PEDANTIC

#if defined(ECXX_ENABLE_ASSERT_PEDANTIC) || defined(ECXX_ENABLE_ASSERT)

#include <cassert>

#endif

#ifdef ECXX_ENABLE_ASSERT
#define ECXX_ASSERT(x) assert(x)
#else
#define ECXX_ASSERT(ignore) ((void)0)
#endif

#ifdef ECXX_ENABLE_ASSERT_PEDANTIC
#define ECXX_FULL_ASSERT(x) assert(x)
#else
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