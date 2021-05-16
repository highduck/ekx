#pragma once

#include "Clock.hpp"

#define SOKOL_TIME_IMPL
#include <sokol_time.h>

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

}