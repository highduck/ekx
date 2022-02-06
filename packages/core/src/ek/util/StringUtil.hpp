#pragma once

#include <ek/ds/String.hpp>

namespace ek {

// Game utilities
inline String format_time_mm_ss(int seconds, uint32_t flags = 0) {
    char buf[32];
    ek_cstr_format_timer(buf, 32, seconds * 1000, (int) flags);
    return {buf};
}

inline String format_time_mm_ss(float seconds, uint32_t flags = 0) {
    char buf[32];
    ek_cstr_format_timer(buf, 32, (int) (seconds * 1000), (int) flags);
    return {buf};
}

}