#pragma once

#include "Stopwatch.hpp"
// TODO: move timers from base library?
#include <sokol_time.h>

namespace ek {

Stopwatch::Stopwatch() {
    initial_ = stm_now();
}

float Stopwatch::readSeconds() const {
    return stm_sec(stm_diff(stm_now(), initial_));
}

float Stopwatch::readMillis() const {
    return stm_ms(stm_diff(stm_now(), initial_));
}

void Stopwatch::reset() {
    initial_ = stm_now();
}

}