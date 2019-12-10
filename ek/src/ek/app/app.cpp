#include "app.hpp"

#include <platform/analytics.hpp>
#include <platform/crash_reporter.hpp>

namespace ek::app {

app_state g_app{};

void dispatch_init() {
    analytics::init(); // analytics before crash reporter on ios
    init_crash_reporter();
}

void dispatch_device_ready() {
    g_app.on_device_ready();
}

void process_event(const event_t& event) {
    g_app.on_event(event);
}

void dispatch_event(const event_t& event) {
    if (event.type == event_type::app_pause) {
        process_event(event);
    } else {
        g_app.event_queue_.emplace_back(event);
    }
}

void dispatch_draw_frame() {
    if (g_app.size_changed) {
        g_app.size_changed = false;
        process_event({event_type::app_resize});
    }
    for (const auto& event : g_app.event_queue_) {
        process_event(event);
    }
    g_app.event_queue_.clear();
    g_app.on_frame_draw();
    g_app.on_frame_completed();
}

}