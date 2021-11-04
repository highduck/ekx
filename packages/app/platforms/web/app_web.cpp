#include <ek/app/app.hpp>

#include <emscripten.h>

// implementation
#include "impl/device_web.h"
#include "impl/sharing_web.h"

extern "C" {
extern void ekapp_setMouseCursor(int cur);
extern bool ekapp_init(int flags);
extern void ekapp_run();
}

using namespace ek::app;

void update_mouse_cursor() {
    if (g_app.dirtyCursor) {
        g_app.dirtyCursor = false;
        ekapp_setMouseCursor((int) g_app.cursor);
    }
}

extern "C" {

EMSCRIPTEN_KEEPALIVE bool _ekapp_onKey(int type, int code, int modifiers) {
    processEvent(KeyEvent{
            static_cast<EventType>(type),
            static_cast<KeyCode>(code),
            static_cast<KeyModifier>(modifiers)
    });
    return true;
}

EMSCRIPTEN_KEEPALIVE bool _ekapp_onMouse(int type, int button, float x, float y) {
    const auto dpr = g_app.dpiScale;
    MouseEvent ev{};
    ev.type = static_cast<EventType>(type);
    ev.button = static_cast<MouseButton>(button);
    ev.x = dpr * x;
    ev.y = dpr * y;
    processEvent(ev);
    return true;
}

EMSCRIPTEN_KEEPALIVE bool _ekapp_onWheel(float x, float y) {
    processEvent(WheelEvent{x, y});
    return true;
}

EMSCRIPTEN_KEEPALIVE bool _ekapp_onTouch(int type, int id, float x, float y) {
    const float dpr = g_app.dpiScale;
    TouchEvent ev{};
    ev.type = static_cast<EventType>(type);
    ev.id = id;
    ev.x = dpr * x;
    ev.y = dpr * y;
    processEvent(ev);
    return true;
}

EMSCRIPTEN_KEEPALIVE bool _ekapp_onResize(float dpr, float w, float h, float dw, float dh) {
    if (g_app.dpiScale != dpr ||
        g_app.windowWidth != w ||
        g_app.windowHeight != h ||
        g_app.drawableWidth != dw ||
        g_app.drawableHeight != dh) {
        g_app.dirtySize = true;

        g_app.dpiScale = dpr;
        g_app.windowWidth = w;
        g_app.windowHeight = h;
        g_app.drawableWidth = dw;
        g_app.drawableHeight = dh;
    }
    return true;
}

EMSCRIPTEN_KEEPALIVE void _ekapp_onFocus(int flags) {
    EventType type;
    if ((flags & 3) == 3) {
        type = EventType::Resume;
    } else {
        type = EventType::Pause;
    }
    processEvent(Event{type});
}

EMSCRIPTEN_KEEPALIVE void _ekapp_loop() {
    processFrame();
    update_mouse_cursor();
    if (g_app.exitRequired) {
        EM_ASM(window.close());
    }
}

}

int main(int argc, char* argv[]) {
    using namespace ek::app;
    g_app.argc = argc;
    g_app.argv = argv;
    ::ek::app::main();
    if (g_app.exitRequired) {
        return g_app.exitCode;
    }

    int flags = 0;
    if (g_app.config.needDepth) {
        flags |= 1;
    }
    if (!ekapp_init(flags)) {
        return 1;
    }

    notifyReady();
    ekapp_run();
    return 0;
}