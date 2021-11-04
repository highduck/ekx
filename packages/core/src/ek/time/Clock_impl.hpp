#pragma once

#include "Clock.hpp"

#define SOKOL_TIME_IMPL
#include <sokol_time.h>

#include <ctime>

namespace ek {

void Clock::setup() {
    stm_setup();
}

double Clock::now() {
    return stm_sec(stm_now());
}

uint64_t Clock::now64() {
    return stm_now();
}

int64_t Clock::unixTimeSeconds() {
    union {
        time_t time;
        int64_t i64;
    } bc{};
    bc.time = time(nullptr);
    return bc.i64;
}

}