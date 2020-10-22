#include <cstdint>

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>

#elif defined(__APPLE__)

#include <mach/mach_time.h>
#include <sys/time.h>

#elif defined(__linux__)

#include <time.h>
#include <sys/time.h>

#elif defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#define OS_WINDOWS

#endif

uint64_t get_time_frequency() {
#if defined(__linux__) || defined(__APPLE__)
    return 1000000;
#elif defined(OS_WINDOWS)
    return 1000;
#elif defined(__EMSCRIPTEN__)
    return 1;
#endif
}

uint64_t get_time_counter() {
#if defined(__EMSCRIPTEN__)
    return 0;
#elif defined(__linux__) || defined(__APPLE__)
    struct timeval tv = {};
    gettimeofday(&tv, NULL);
    return (uint64_t) tv.tv_sec * (uint64_t) 1000000 + (uint64_t) tv.tv_usec;
#elif defined(OS_WINDOWS)
    return (uint64_t) timeGetTime();
#endif
}

uint64_t query_performance_frequency() {
#if defined(__EMSCRIPTEN__)
    return 1000000000;
#elif defined(__APPLE__)
    mach_timebase_info_data_t info;
    if (mach_timebase_info(&info) == 0) {
        return (uint64_t) ((info.denom * 1e9) / info.numer);
    }
#elif defined(__linux__) && defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return 1000000000;
    }
#elif defined(OS_WINDOWS)
    uint64_t value;
    if (QueryPerformanceFrequency((LARGE_INTEGER*) &value)) {
        return value;
    }
#endif
    return 0;
}

uint64_t query_performance_counter() {
#if defined(__EMSCRIPTEN__)
    return static_cast<uint64_t>(emscripten_get_now() * 1000000.0);
#elif defined(__APPLE__)
    return mach_absolute_time();
#elif defined(__linux__) && defined(CLOCK_MONOTONIC)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * (uint64_t) 1000000000 + (uint64_t) ts.tv_nsec;
#elif defined(OS_WINDOWS)
    uint64_t value;
    QueryPerformanceCounter((LARGE_INTEGER*) &value);
    return value;
#else
    return 0;
#endif
}

typedef uint64_t (* counter_func)();

struct clock_state_t {
    counter_func query_counter;
    uint64_t frequency;
    uint64_t startup;
};

clock_state_t init_clock() {
    auto freq = query_performance_frequency();
    auto func = query_performance_counter;
    if (!freq) {
        freq = get_time_frequency();
        func = get_time_counter;
    }
    return {func, freq, func()};
}

namespace ek::clock {

double now() {
    static auto st = init_clock();
    return static_cast<double>(st.query_counter() - st.startup) / st.frequency;
}

}
