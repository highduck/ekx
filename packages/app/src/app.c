#include "intern.h"
#include <ek/log.h>
#include <ek/assert.h>
#include <string.h>

ek_app_context ek_app;

void ek_app_set_mouse_cursor(ek_mouse_cursor cursor) {
    if (ek_app.cursor != cursor) {
        ek_app.cursor = cursor;
        ek_app.state |= EK_APP_STATE_CURSOR_CHANGED;
    }
}

void ek_app_quit(int status) {
    ek_app.state |= EK_APP_STATE_EXIT_PENDING;
    ek_app.exit_code = status;
}

void ek_app_cancel_quit(void) {
    ek_app.state = (ek_app.state & (~EK_APP_STATE_EXIT_PENDING));
}

void ek_app__init(void) {
    memset(&ek_app, 0, sizeof(ek_app_context));
    ek_app.viewport.width = 1.0f;
    ek_app.viewport.height = 1.0f;
    ek_app.viewport.scale = 1.0f;

    ek_app.config.title = "";
    ek_app.config.width = 960;
    ek_app.config.height = 720;
    ek_app.config.allow_high_dpi = true;
    ek_app.config.sample_count = 1;
    ek_app.config.swap_interval = 1;
}

static bool handle_pause_resume(const ek_app_event_type event_type) {
    // filter pause/resume events according to application focus state
    if (event_type == EK_APP_EVENT_PAUSE) {
        if (ek_app.state & EK_APP_STATE_RUNNING) {
            ek_app.state ^= EK_APP_STATE_RUNNING;
        } else {
            // do not fire `pause` event
            return false;
        }
    } else if (event_type == EK_APP_EVENT_RESUME) {
        if (!(ek_app.state & EK_APP_STATE_RUNNING)) {
            ek_app.state ^= EK_APP_STATE_RUNNING;
        } else {
            // do not fire `resume` event
            return false;
        }
    }
    return true;
}

void ek_app__process_event(const ek_app_event event) {
    EK_ASSERT((uint32_t) event.type < EK_APP_EVENT_COUNT);

    if (!handle_pause_resume(event.type)) {
        return;
    }

    if (ek_app.on_event) {
        ek_app.on_event(event);
    }
}

void ek_app__process_frame(void) {
    if (ek_app.on_frame) {
        if (ek_app.state & EK_APP_STATE_SIZE_CHANGED) {
            ek_app.state ^= EK_APP_STATE_SIZE_CHANGED;
            ek_app__process_event((ek_app_event) {
                    .type = EK_APP_EVENT_RESIZE
            });
        }
        ek_app.on_frame();
    }
}

void ek_app__notify_ready() {
    log_debug("ekapp ready");
    EK_ASSERT((ek_app.state & EK_APP_STATE_READY) == 0);
    ek_app.state |= EK_APP_STATE_READY;
    if (ek_app.on_ready) {
        ek_app.on_ready();
    }
}

void ek_app__update_viewport(ek_app_viewport viewport) {
    if (0 != memcmp(&ek_app.viewport, &viewport, sizeof(ek_app_viewport))) {
        memcpy(&ek_app.viewport, &viewport, sizeof(ek_app_viewport));
        ek_app.state |= EK_APP_STATE_SIZE_CHANGED;
    }
}

#if defined(__EMSCRIPTEN__)
#include "web/web.c.h"
#elif defined(__ANDROID__)
#include "android/android.c.h"
#elif defined(__APPLE__)
// separated obj-c implementation in `apple.m` file
#else
#include "null/app-null.c.h"
#endif

