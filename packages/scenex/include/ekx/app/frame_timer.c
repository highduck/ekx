#include "frame_timer.h"
#include <ek/app.h>
#include <ek/time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

static bool frame_timer_update_from_display_ts(struct frame_timer_t* ft, double* delta) {
    if (ek_app.frame_callback_timestamp > 0) {
        const double ts = ek_app.frame_callback_timestamp;
        const double prev = ft->app_fts_prev;
        ft->app_fts_prev = ts;
        if (prev > 0.0) {
            *delta = ts - prev;
            return true;
        }
    }
    return false;
}

double update_frame_timer(frame_timer_t* timer) {
    // anyway we need update stopwatch state, it could be useful for another functions
    double dt = ek_ticks_to_sec(ek_ticks(&timer->stopwatch));
    // if available, upgrade delta with timestamp from app's display-link
    frame_timer_update_from_display_ts(timer, &dt);
    timer->dt = dt;
    ++timer->idx;
    return dt;
}

#ifdef __cplusplus
}
#endif
