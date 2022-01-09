#pragma once

#include <ek/assert.h>
#include <ek/rnd.h>
#include <ek/time.h>
#include <ek/math.h>

namespace ek {

struct InitRandomized {
};

class Random final {
public:

    constexpr Random() noexcept: state{0} {}

    Random(InitRandomized) noexcept: state{ek_time_seed32()} {}

    uint32_t roll(uint32_t max_exclusive) {
        return next() % max_exclusive;
    }

    uint32_t next() {
        return ek_rand1(&state);
    }

    void setSeed(uint32_t seed) {
        state = seed;
    }

    [[nodiscard]]
    uint32_t getSeed() const {
        return state;
    }

    float random() {
        return ek_norm_f32_from_u32(next());
    }

    float random(float min, float max) {
        return lerp_f32(min, max, random());
    }

    int random_int(int min, int max) {
        EK_ASSERT(max >= min);
        const auto range = static_cast<uint32_t>(max - min);
        return min + (int) roll(range + 1);
    }

    bool chance(float prob = 0.5f) {
        float r = random();
//        log_debug("%d < %d => %d", (int)(r * 1000), (int)(prob * 1000), r < prob ? 1 : 0);
        return r < prob;
    }

    template<typename Container, typename T = typename Container::value_type>
    inline T random_element(const Container& array) {
        const auto sz = static_cast<uint32_t>(array.size());
        return sz > 0 ? array[roll(sz)] : T{};
    }

    uint32_t state = 0;
//    uint64_t state = 0;
};

inline Random rand_default;
inline Random rand_game;
inline Random rand_fx;

inline float random() {
    return rand_default.random();
}

inline float random(float min, float max) {
    return rand_default.random(min, max);
}

inline int random_int(int min, int max) {
    return rand_default.random_int(min, max);
}

inline bool random_chance(float prob = 0.5f) {
    return rand_default.chance(prob);
}

template<typename Container, typename T = typename Container::value_type>
T random_element(const Container& array) {
    return rand_default.random_element(array);
}

}