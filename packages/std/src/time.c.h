#include <ek/time.h>

#define SOKOL_TIME_IMPL

#include <sokol_time.h>

#include <time.h>

void ek_time_init(void) {
    stm_setup();
}

double ek_time_now(void) {
    return stm_sec(stm_now());
}

uint64_t ek_ticks(uint64_t* prev_ticks) {
    uint64_t n = stm_now();
    uint64_t t;
    if (prev_ticks) {
        t = stm_diff(n, *prev_ticks);
        *prev_ticks = n;
    } else {
        t = n;
    }
    return t;
}

double ek_ticks_to_sec(uint64_t ticks) {
    return stm_sec(ticks);
}

// unix UTC in seconds
uint64_t ek_time_utc(void) {
    union {
        time_t time;
        uint64_t u64;
    } bc;
    bc.time = time(0);
    return bc.u64;
}

uint32_t ek_time_seed32(void) {
    return (uint32_t) (ek_time_utc() ^ ek_ticks(0));
}
