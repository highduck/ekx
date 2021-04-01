#pragma once

#include "common.hpp"
#include "../assert.hpp"
#include <cstdint>

namespace ek {

class Lcg32 {
    static constexpr uint32_t A = 1103515245u;
    static constexpr uint32_t C = 12345u;
    static constexpr uint32_t M = (1u << 31u) - 1;

public:

    inline Lcg32() noexcept {
        // Kind hack:
        // 1) seed is uninitialized
        // 2) normalize seed state
        // seed_ = seed_ & M;

        // use this pointer address as initial state:
        // - kindly undefined
        // - static memory range could not give us full range...
        //seed_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this) & M);

        seed_ = static_cast<uint32_t>(seed_ ^ reinterpret_cast<uintptr_t>(this)) & M;
    }

    inline explicit Lcg32(uint32_t seed) : seed_{seed} {}

    inline uint32_t next() {
        seed_ = (seed_ * A + C) & M;
        return seed_;
    }

    [[nodiscard]]
    inline uint32_t seed() const {
        return seed_;
    }

    inline void seed(uint32_t v) {
        seed_ = v;
    }

private:
    uint32_t seed_;
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

    inline void set_seed(uint32_t seed) {
        engine.seed(seed);
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