#ifndef EKX_APP_FRAME_TIMER_H
#define EKX_APP_FRAME_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct frame_timer_t {
    // delta time between frames in seconds
    double dt;
    // frame index, continuously growing, theoretically useful to check frame-stamps
    uint64_t idx;

    double app_fts_prev;
    uint64_t stopwatch;
} frame_timer_t;

double update_frame_timer(frame_timer_t* timer);

#ifdef __cplusplus
}
#endif

#endif // EKX_APP_FRAME_TIMER_H
