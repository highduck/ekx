#include "app.hpp"

namespace ek::app {

void notifyReady() {
    EKAPP_LOG("ekapp ready");
    EKAPP_ASSERT(!g_app.ready);
    g_app.ready = true;
    if (g_app.listener) {
        g_app.listener->onReady();
    }
}

void processEvent(const Event& event) {
    EKAPP_ASSERT(static_cast<uint8_t>(event.type) < static_cast<uint8_t>(EventType::Count));

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
            processEvent(EventType::Resize);
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

const char* findArgumentValue(const char* key, const char* defaultValue) {
    const int argc = g_app.argc;
    char** argv = g_app.argv;
    int keySize = (int) strlen(key);
    for (int i = 0; i < argc; ++i) {
        if (strncmp(key, argv[i], keySize) == 0) {
            ++i;
            if (i < argc) {
                return argv[i];
            }
            return defaultValue;
        }
    }
    return defaultValue;
}

}