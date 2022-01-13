#ifdef __cplusplus

#include <ek/assert.h>

template<typename T>
inline static T rnd_pick_item(uint32_t* rnd, const T* array, uint32_t len) {
    EK_ASSERT(rnd);
    EK_ASSERT(array);
    EK_ASSERT(len);
    // using with small arrays, just to sanitize `len` bad casts
    EK_ASSERT(len < 0x80000000u);
    EK_ASSERT(len < 0x8000u);
    return array[ek_rand1(rnd) % len];
}

template<typename A, typename T = typename A::value_type>
inline static T rnd_pick_item(uint32_t* rnd, const A& array) {
    return rnd_pick_item(rnd, array.data(), array.size());
}

#endif
