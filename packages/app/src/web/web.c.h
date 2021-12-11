#include "app.js.h"

inline static void update_mouse_cursor(void) {
    if (ek_app.state & EK_APP_STATE_CURSOR_CHANGED) {
        ek_app.state ^= EK_APP_STATE_CURSOR_CHANGED;
        ek_app_js_set_mouse_cursor((int) ek_app.cursor);
    }
}

EMSCRIPTEN_KEEPALIVE bool ek_app_js__on_key(int type, int code, int modifiers) {
    ek_app__process_event((ek_app_event) {.key={
            .type = (ek_app_event_type) type,
            .code = (ek_key_code) code,
            .modifiers = (ek_key_mod) modifiers
    }});
    return true;
}

EMSCRIPTEN_KEEPALIVE bool ek_app_js__on_mouse(int type, int button, float x, float y) {
    const float dpr = ek_app.viewport.scale;
    const ek_app_event ev = {.mouse = {
            .type = (ek_app_event_type) type,
            .button = (ek_mouse_button) button,
            .x = dpr * x,
            .y = dpr * y
    }};
    ek_app__process_event(ev);
    return true;
}

EMSCRIPTEN_KEEPALIVE bool ek_app_js__on_wheel(float x, float y) {
    const ek_app_event ev = {.wheel = {
            .type = EK_APP_EVENT_WHEEL,
            .x = x,
            .y = y
    }};
    ek_app__process_event(ev);
    return true;
}

EMSCRIPTEN_KEEPALIVE bool ek_app_js__on_touch(int type, int id, float x, float y) {
    const float dpr = ek_app.viewport.scale;
    const ek_app_event ev = {.touch = {
            .type = (ek_app_event_type) type,
            .id = (uint64_t) id,
            .x = dpr * x,
            .y = dpr * y
    }};
    ek_app__process_event(ev);
    return true;
}

EMSCRIPTEN_KEEPALIVE bool ek_app_js__on_resize(float w, float h, float dpr) {
    ek_app__update_viewport((ek_app_viewport) {
            .width = w,
            .height = h,
            .scale = dpr,
            .insets = {0.0f, 0.0f, 0.0f, 0.0f}
    });
    return true;
}

EMSCRIPTEN_KEEPALIVE void ek_app_js__on_focus(int flags) {
    ek_app_event event;
    if ((flags & 3) == 3) {
        event.type = EK_APP_EVENT_RESUME;
    } else {
        event.type = EK_APP_EVENT_PAUSE;
    }
    ek_app__process_event(event);
}

EMSCRIPTEN_KEEPALIVE void ek_app_js__loop(void) {
    ek_app__process_frame();
    update_mouse_cursor();
    if (ek_app.state & EK_APP_STATE_EXIT_PENDING) {
        ek_app.state ^= EK_APP_STATE_EXIT_PENDING;
        ek_app_js_close();
    }
}

int ek_app_open_url(const char* url) {
    return ek_app_js_navigate(url);
}

int ek_app_font_path(char* dest, uint32_t size, const char* font_name) {
    (void) sizeof(dest);
    (void) sizeof(size);
    (void) sizeof(font_name);
    return 1;
}

int ek_app_share(const char* content) {
    return ek_app_js_share(content);
}

int main(int argc, char* argv[]) {
    ek_app__init();
    ek_app.argc = argc;
    ek_app.argv = argv;
    ek_app_main();
    if (ek_app.state & EK_APP_STATE_EXIT_PENDING) {
        return ek_app.exit_code;
    }

    int flags = 0;
    if (ek_app.config.need_depth) {
        flags |= 1;
    }
    if (!ek_app_js_init(flags)) {
        return 1;
    }
    ek_app_js_lang(ek_app.lang, sizeof(ek_app.lang));

    ek_app__notify_ready();
    ek_app_js_run();
    return 0;
}
