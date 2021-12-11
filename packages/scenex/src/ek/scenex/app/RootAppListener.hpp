#pragma once

#include <ek/app.h>
#include <ek/timers.hpp>
#include <ek/audio/audio.hpp>
#include <ek/log.h>
#include <ek/assert.h>
#include <sokol_gfx.h>

namespace ek {

inline void root_app_on_frame() {
    log_tick();
    dispatchTimers();
}

inline void root_app_on_event(const ek_app_event ev) {
    if (ev.type == EK_APP_EVENT_PAUSE) {
        auph::pause();
    } else if (ev.type == EK_APP_EVENT_RESUME) {
        auph::resume();
    } else if (ev.type == EK_APP_EVENT_CLOSE) {
        auph::shutdown();
        sg_shutdown();
    }
}

}