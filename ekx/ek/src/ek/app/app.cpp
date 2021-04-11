#include "app.hpp"

#include <ek/ext/analytics/analytics.hpp>
#include <mutex>
#include <ek/assert.hpp>
#include <ek/timers.hpp>
#include <ek/audio/audio.hpp>
#include <ek/util/logger.hpp>
#include <ek/util/StaticStorage.hpp>
#include <Tracy.hpp>

namespace ek::app {

std::mutex event_queue_mtx;

static StaticStorage<app_state> ssAppState;
app_state& g_app = *ssAppState.ptr();

void dispatch_init() {
    EK_TRACE << "app on init";
    EK_TRACE << "analytics initialize";
    analytics::init(); // analytics before crash reporter on ios
}

void dispatch_device_ready() {
    EK_TRACE << "app device ready";
    g_app.on_device_ready();
}

void process_event(const event_t& event) {
    EK_ASSERT(static_cast<uint8_t>(event.type) < static_cast<uint8_t>(event_type::max_count));

    switch (event.type) {
        case event_type::app_pause:
            if (g_app.applicationInFocus) {
                g_app.applicationInFocus = false;
                g_app.on_event(event);
            }
            break;
        case event_type::app_resume:
            if (!g_app.applicationInFocus) {
                g_app.applicationInFocus = true;
                g_app.on_event(event);
            }
            break;
        default:
            g_app.on_event(event);
            break;
    }
}

void dispatch_event(const event_t& event) {
    EK_ASSERT(static_cast<uint8_t>(event.type) < static_cast<uint8_t>(event_type::max_count));
//    EK_INFO("EVENT: %i", event.type);
    if (event.type == event_type::app_pause) {
        ++g_app.systemPauseCounter;
        if (g_app.systemPauseCounter > 0 && !g_app.systemPaused) {
            g_app.systemPaused = true;
            audio::muteDeviceBegin();
        }
    } else if (event.type == event_type::app_resume) {
        if (g_app.systemPauseCounter > 0) {
            --g_app.systemPauseCounter;
            if (g_app.systemPauseCounter == 0 && g_app.systemPaused) {
                g_app.systemPaused = false;
                audio::muteDeviceEnd();
            }
        }
    }

    {
        event_queue_mtx.lock();
        assert(!g_app.event_queue_locked);
        g_app.event_queue_.push_back(event);
        event_queue_mtx.unlock();
    }
}

void dispatch_draw_frame() {
    tracy::SetThreadName("Render");
    FrameMark;
    logger::nextFrame();

    event_queue_mtx.lock();
    // do not call dispatch_draw_frame recursively
    EK_ASSERT(!g_app.event_queue_locked);
    g_app.event_queue_locked = true;
    g_app.pool_queue = g_app.event_queue_;
    g_app.event_queue_.clear();
    g_app.event_queue_locked = false;
    event_queue_mtx.unlock();

    if (g_app.size_changed) {
        g_app.size_changed = false;
        process_event({event_type::app_resize});
    }

    for (const auto& event : g_app.pool_queue) {
        process_event(event);
    }

    dispatchTimers();

    g_app.on_frame_draw();
    g_app.on_frame_completed();

//    if (g_app.systemPauseCounter > 0 && !g_app.systemPaused) {
//        g_app.systemPaused = true;
//        process_event({event_type::app_pause});
//    }
//    else if(g_app.systemPauseCounter == 0 && g_app.systemPaused) {
//        g_app.systemPaused = false;
//        process_event({event_type::app_resume});
//    }
}

std::vector<std::string> arguments::to_vector() const {
    using namespace std;
    vector<string> result{};
    result.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        result.emplace_back(argv[i]);
    }
    return result;
}

void app_state::updateMouseCursor(mouse_cursor cursor_) {
    if (cursor != cursor_) {
        cursor = cursor_;
        cursor_dirty = true;
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