#include "app.hpp"

#include <mutex>
#include <ek/assert.hpp>
#include <ek/timers.hpp>
#include <ek/audio/audio.hpp>
#include <ek/debug.hpp>
#include <ek/util/StaticStorage.hpp>
#include <Tracy.hpp>
#include <ek/debug/LogSystem.hpp>

namespace ek::app {

std::mutex event_queue_mtx;

static StaticStorage<App> ssAppState;
App& g_app = *ssAppState.ptr();

void dispatch_init() {
    EK_TRACE << "App: pre-start";
    EK_ASSERT(!g_app.preStarted);
    g_app.preStarted = true;
    if (g_app.listener) {
        g_app.listener->onPreStart();
    }
}

void dispatch_device_ready() {
    EK_TRACE << "App: device ready";
    EK_ASSERT(!g_app.ready);
    g_app.ready = true;
    if (g_app.listener) {
        g_app.listener->onDeviceReady();
    }
}

void process_event(const Event& event) {
    EK_ASSERT(static_cast<uint8_t>(event.type) < static_cast<uint8_t>(Event::Count));

    // filter pause/resume events according to application focus state
    if (event.type == Event::Pause) {
        if (!g_app.running) {
            return;
        }
        g_app.running = false;
    } else if (event.type == Event::Resume) {
        if (g_app.running) {
            return;
        }
        g_app.running = true;
    }

    if (g_app.listener) {
        g_app.listener->onEvent(event);
    }
}

void dispatch_event(const Event& event) {
    EK_ASSERT(static_cast<uint8_t>(event.type) < static_cast<uint8_t>(Event::Count));
    if (event.type == Event::Pause) {
        auph::pause();
    } else if (event.type == Event::Resume) {
        auph::resume();
    }

    event_queue_mtx.lock();
    EK_ASSERT(!g_app.eventQueueLocked);
    g_app.eventQueue.push_back(event);
    event_queue_mtx.unlock();
}

void dispatch_draw_frame() {
    tracy::SetThreadName("Render");
    FrameMark;
    LogSystem::instance().nextFrame();

    event_queue_mtx.lock();
    // do not call dispatch_draw_frame recursively
    EK_ASSERT(!g_app.eventQueueLocked);
    g_app.eventQueueLocked = true;
    g_app.eventQueuePool = g_app.eventQueue;
    g_app.eventQueue.clear();
    g_app.eventQueueLocked = false;
    event_queue_mtx.unlock();

    if (g_app.dirtySize) {
        g_app.dirtySize = false;
        Event ev{};
        ev.type = Event::Resize;
        process_event(ev);
    }

    for (const auto& event : g_app.eventQueuePool) {
        process_event(event);
    }

    dispatchTimers();

    if (g_app.listener) {
        g_app.listener->onFrame();
        g_app.listener->onPostFrame();
    }
}

void App::updateMouseCursor(MouseCursor cursor_) {
    if (cursor != cursor_) {
        cursor = cursor_;
        dirtyCursor = true;
    }
}

void initialize() {
    EK_TRACE << "app initialize";
    ssAppState.initialize();
}

void shutdown() {
    EK_TRACE << "app shutdown";
    ssAppState.shutdown();
}

}