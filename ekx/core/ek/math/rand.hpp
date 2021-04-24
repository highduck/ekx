#pragma once

#include "common.hpp"
#include "../assert.hpp"
#include <cstdint>

namespace ek {

uint32_t generateRandomSeedFromTime(uint32_t mask);

struct Lcg32 {
    static constexpr uint32_t A = 1103515245u;
    static constexpr uint32_t C = 12345u;
    static constexpr uint32_t M = (1u << 31u) - 1;

    uint32_t seed;

    inline Lcg32() noexcept: seed{generateRandomSeedFromTime(M)} {}

    inline explicit Lcg32(uint32_t initSeed) : seed{initSeed} {}

    inline uint32_t next() {
        seed = (seed * A + C) & M;
        return seed;
    }
};

template<class Engine = Lcg32>
class Random {

    static constexpr uint32_t max_float_mask = 0x00FFFFFFu;

public:

    inline uint32_t roll(uint32_t max_exclusive) {
        return next() % max_exclusive;
    }

    inline uint32_t next() {
        return engine.next();
    }

    inline void setSeed(uint32_t seed) {
        engine.seed = seed;
    }

    [[nodiscard]]
    inline uint32_t getSeed() const {
        return engine.seed;
    }

    inline float random() {
        return static_cast<float>(next() & max_float_mask) / max_float_mask;
    }

    inline float random(float min, float max) {
        return math::lerp(min, max, random());
    }

    inline int random_int(int min, int max) {
        EK_ASSERT(max >= min);
        const auto range = static_cast<uint32_t>(max - min);
        return min + roll(range + 1);
    }

    inline bool chance(float prob = 0.5f) {
        return random() <= prob;
    }

    template<typename Container, typename T = typename Container::value_type>
    inline T random_element(const Container& array) {
        const auto size = static_cast<uint32_t>(array.size());
        return size > 0 ? array[roll(size)] : T{};
    }

    inline void randomizeFromTime() {
        engine.randomizeFromTime();
    }

    Engine engine{};
};

extern Random<Lcg32> rand_default;
extern Random<Lcg32> rand_game;
extern Random<Lcg32> rand_fx;

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