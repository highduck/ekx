#include "app.hpp"

#include <platform/analytics.hpp>
#include <platform/crash_reporter.hpp>

namespace ek {

application_t g_app{};

application_listener_t::~application_listener_t() = default;

void dispatch_init() {
    analytics::init(); // analytics before crash reporter on ios
    init_crash_reporter();
}

void dispatch_device_ready() {
    ek_main();
}

void application_t::handle_event(const event_t& event) {
    for (auto* listener : listeners_) {
        listener->on_event(event);
    }
}

void application_t::dispatch_draw_frame() {
    if (g_app.size_changed) {
        g_app.size_changed = false;
        handle_event({event_type::app_resize});
    }
    for (const auto& event : event_queue_) {
        handle_event(event);
    }
    event_queue_.clear();

    for (auto* listener : listeners_) {
        listener->on_draw_frame();
    }

    for (auto* listener : listeners_) {
        listener->on_frame_completed();
    }
}

}