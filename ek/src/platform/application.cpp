#include "application.hpp"
#include "analytics.hpp"
#include "crash_reporter.hpp"

#include <utility>

namespace ek {

application_t g_app{};

void setup_application_window(window_creation_config_t config) {
    g_window.creation_config = std::move(config);
}

application_listener_t::~application_listener_t() = default;

void application_t::init() {
    analytics::init(); // analytics before crash reporter on ios
    init_crash_reporter();
}

void application_t::start() {
    ek_main();
}

void application_t::handle_event(const event_t& event) {
    for (auto* listener : listeners_) {
        listener->on_event(event);
    }
}

void application_t::dispatch_draw_frame() {
    if (g_window.size_changed) {
        g_window.size_changed = false;
        event_t ev{event_type::app_resize};
        ev.x = g_window.back_buffer_size.width;
        ev.y = g_window.back_buffer_size.height;
        handle_event(ev);
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