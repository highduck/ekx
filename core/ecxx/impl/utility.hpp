#pragma once

namespace ecs::details {

constexpr unsigned bit_count(const unsigned long long x) noexcept {
    return (x == 0u) ? 0u : ((x & 1u) + bit_count(x >> 1u));
}

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