#pragma once

#include <ek/ds/String.hpp>

namespace ek {

enum TimeFormatFlags {
    TimeFormat_None = 0,
    TimeFormat_KeepHours = 1
};

// Game utilities
inline String format_time_mm_ss(int seconds, TimeFormatFlags flags = TimeFormat_None) {
    char buf[32];
    ek_cstr_format_timer(buf, 32, seconds * 1000, (int) flags);
    return {buf};
}

inline String format_time_mm_ss(float seconds, TimeFormatFlags flags = TimeFormat_None) {
    char buf[32];
    ek_cstr_format_timer(buf, 32, (int) (seconds * 1000), (int) flags);
    return {buf};
}

}