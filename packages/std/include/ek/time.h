#ifndef EK_TIME_H
#define EK_TIME_H

#include <ek/pre.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint64_t ek_ticks_from_seconds(double seconds) {
    return (uint64_t) (seconds * 1e9);
}
double ek_ticks_to_sec(uint64_t ticks);

void ek_time_init(void);

double ek_time_now(void);

uint64_t ek_ticks(uint64_t* prev_ticks);

// unix UTC in seconds
uint64_t ek_time_utc(void);

uint32_t ek_time_seed32(void);

// Timer System
void ek_timers_init(void);

void ek_timers_update();

typedef struct ek_timer_callback {
    void (* action)(void* userdata);

    void (* cleanup)(void* userdata);

    void* userdata;
} ek_timer_callback;

typedef struct ek_timer_desc {
    uint64_t interval;
    uint64_t delay;
    int calls;
    ek_timer_callback callback;
} ek_timer_desc;

typedef struct ek_timer {
    int id;
} ek_timer;

enum {
    EK_TIMER_REPEAT_INFINITE = -1
};

ek_timer ek_timer_start(ek_timer_desc desc);
ek_timer ek_set_timeout(ek_timer_callback callback, double seconds);
ek_timer ek_set_interval(ek_timer_callback callback, double seconds);
bool ek_timer_stop(ek_timer timer);

#ifdef __cplusplus
}
#endif

#endif //EK_TIME_H
