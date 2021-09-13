#include "app.hpp"

#include <ek/assert.hpp>
#include <ek/debug.hpp>
#include <ek/debug/LogSystem.hpp>

namespace ek::app {

void notifyInit() {
    EK_TRACE << "App: initialize";
    EK_ASSERT(!g_app.initialized);
    g_app.initialized = true;
    if (g_app.listener) {
        g_app.listener->onInitialize();
    }
}

void notifyReady() {
    EK_TRACE << "App: device ready";
    EK_ASSERT(!g_app.ready);
    g_app.ready = true;
    if (g_app.listener) {
        g_app.listener->onReady();
    }
}

void processEvent(const Event& event) {
    EK_ASSERT(static_cast<uint8_t>(event.type) < static_cast<uint8_t>(EventType::Count));

    // filter pause/resume events according to application focus state
    if (event.type == EventType::Pause) {
        if (!g_app.running) {
            return;
        }
        g_app.running = false;
    } else if (event.type == EventType::Resume) {
        if (g_app.running) {
            return;
        }
        g_app.running = true;
    }

    if (g_app.listener) {
        g_app.listener->onEvent(event);
    }
}

void processFrame() {
    if (g_app.listener) {
        if (g_app.dirtySize) {
            g_app.dirtySize = false;
            Event ev{};
            ev.type = EventType::Resize;
            processEvent(ev);
        }
        g_app.listener->onFrame();
    }
}

void setMouseCursor(MouseCursor cursor_) {
    if (g_app.cursor != cursor_) {
        g_app.cursor = cursor_;
        g_app.dirtyCursor = true;
    }
}

void quit(int status) {
    g_app.exitRequired = true;
    g_app.exitCode = status;
}

void cancelQuit() {
    g_app.exitRequired = false;
}

}