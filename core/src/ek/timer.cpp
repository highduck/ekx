#include "timer.hpp"

#if defined(__APPLE__)

#include <mach/mach_time.h>

#elif defined(__linux__)

#include <time.h>

#elif defined(__EMSCRIPTEN__)

#include <emscripten.h>

#elif defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <timeapi.h>

#endif

#if defined(__APPLE__) || defined(__linux__)

#include <sys/time.h>

#endif

namespace ek {

namespace {

using counter_type = clock_device_t::counter_type;
using frequency_type = clock_device_t::frequency_type;

inline frequency_type get_time_frequency() {
#if defined(__linux__) || defined(__APPLE__)
    return 1000000;
#elif defined(_WIN32)
    return 1000;
#elif defined(__EMSCRIPTEN__)
    return 1000000000;
#endif
}

inline counter_type get_time_counter() {
#if defined(__linux__) || defined(__APPLE__)
    timeval tv{};
    gettimeofday(&tv, nullptr);
    return (uint64_t) tv.tv_sec * (uint64_t) 1000000 + (uint64_t) tv.tv_usec;
#elif defined(_WIN32)
    return (uint64_t) timeGetTime();
#elif defined(__EMSCRIPTEN__)
    return (uint64_t)(emscripten_get_now() * 1000000);
#endif
}


frequency_type query_performance_frequency() {
#if defined(__APPLE__)
    mach_timebase_info_data_t info;
    if (mach_timebase_info(&info) == 0) {
        return (uint64_t) ((info.denom * 1e9) / info.numer);
    }
#elif defined(__linux__) && defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return 1000000000;
    }
#elif defined(_WIN32)
    uint64_t value;
    if (QueryPerformanceFrequency((LARGE_INTEGER*) &value)) {
        return value;
    }
#endif
    return 0;
}

inline counter_type query_performance_counter() {
#if defined(__APPLE__)
    return mach_absolute_time();
#elif defined(__linux__) && defined(CLOCK_MONOTONIC)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * (uint64_t) 1000000000 + (uint64_t) ts.tv_nsec;
#elif defined(_WIN32)
    uint64_t value;
    QueryPerformanceCounter((LARGE_INTEGER*) &value);
    return value;
#else
    return 0;
#endif
}
}

clock_device_t::clock_device_t() noexcept {
    const auto hp_freq = query_performance_frequency();
    if (hp_freq != 0u) {
        // high precision timer
        query_counter_func_ = &query_performance_counter;
        frequency_ = hp_freq;
    } else {
        query_counter_func_ = &get_time_counter;
        frequency_ = get_time_frequency();
    }
}

static clock_device_t global_clock_device_{};

const clock_device_t& get_clock_device() {
    return global_clock_device_;
}
}
