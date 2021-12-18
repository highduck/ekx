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


// Timer System

typedef struct ek_timer_state {
    int id;
    // 0 - stop, -1 = infinite
    int remaining_calls;
    uint64_t interval;
    uint64_t next_call_ts;
    ek_timer_callback callback;
} ek_timer_state;

static ek_timer_state ek_timers[64];
static int ek_timers_next_id;
static int ek_timers_length;

void ek_timers_init(void) {
    ek_timers_next_id = 0;
    ek_timers_length = 0;
    memset(ek_timers, 0, sizeof(ek_timers));
}

void ek_timers_update() {
    uint64_t now = ek_ticks(0);
    for (int i = 0; i < ek_timers_length; ++i) {
        ek_timer_state* state = ek_timers + i;
        if(state->remaining_calls) {
            if (now >= state->next_call_ts) {
                ek_timer_callback cb = state->callback;
                if(cb.action) {
                    cb.action(cb.userdata);
                }
                if(state->remaining_calls > 0) {
                    --state->remaining_calls;
                }
                state->next_call_ts += state->interval;
            }
        }
    }

    for(int i = 0; i < ek_timers_length;) {
        ek_timer_state* state = ek_timers + i;
        if(state->remaining_calls) {
            ++i;
        }
        else {
            ek_timer_callback cb = state->callback;
            if(cb.cleanup) {
                cb.cleanup(cb.userdata);
            }
            --ek_timers_length;
            if(i < ek_timers_length) {
                ek_timers[i] = ek_timers[ek_timers_length];
            }
        }
    }
}

ek_timer ek_timer_start(ek_timer_desc desc) {
    EK_ASSERT(ek_timers_length < 64);
    ek_timer_state* state = ek_timers + ek_timers_length;
    ++ek_timers_length;
    state->id = ++ek_timers_next_id;
    state->remaining_calls = desc.calls;
    state->interval = desc.interval;
    state->next_call_ts = ek_ticks(0) + desc.delay;
    state->callback = desc.callback;

    return (ek_timer){.id = state->id};
}

bool ek_timer_stop(ek_timer timer) {
    for(int i = 0; i < ek_timers_length; ++i) {
        if(ek_timers[i].id == timer.id && ek_timers[i].remaining_calls) {
            ek_timers[i].remaining_calls = 0;
            return true;
        }
    }
    return false;
}

ek_timer ek_set_timeout(ek_timer_callback callback, double seconds) {
    return ek_timer_start((ek_timer_desc) {
        .interval = 0,
        .delay = ek_ticks_from_seconds(seconds),
        .calls = 1,
        .callback = callback
    });
}

ek_timer ek_set_interval(ek_timer_callback callback, double seconds) {
    const uint64_t ticks = ek_ticks_from_seconds(seconds);
    return ek_timer_start((ek_timer_desc) {
            .interval = ticks,
            .delay = ticks,
            .calls = -1,
            .callback = callback
    });
}