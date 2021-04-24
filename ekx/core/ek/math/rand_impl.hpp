#pragma once

#include "rand.hpp"
#include <ctime>

namespace ek {

Random<Lcg32> rand_default{};
Random<Lcg32> rand_game{};
Random<Lcg32> rand_fx{};

uint32_t generateRandomSeedFromTime(uint32_t mask) {
    static uint32_t offset = 0;
    const auto unixTimeStamp = time(nullptr);
    const auto step = offset++;
    return static_cast<uint32_t>((unixTimeStamp + step) & mask);
}

}