#pragma once

#include "basic_application.hpp"
#include <ek/canvas.h>

namespace ek {

void root_app_on_frame() {
    log_tick();
    ek_timers_update();
}

void root_app_on_event(const ek_app_event ev) {
    if (ev.type == EK_APP_EVENT_PAUSE) {
        auph_set_pause(AUPH_MIXER, true);
    } else if (ev.type == EK_APP_EVENT_RESUME) {
        auph_set_pause(AUPH_MIXER, false);
    } else if (ev.type == EK_APP_EVENT_CLOSE) {
        ek_app.on_frame = nullptr;
        ek_app.on_event = nullptr;
        if(g_game_app) {
            g_game_app->terminate();
            delete g_game_app;
            g_game_app = nullptr;
        }
        ecx_shutdown();
        canvas_shutdown();
        auph_shutdown();
        ek_gfx_shutdown();
    }
}

}