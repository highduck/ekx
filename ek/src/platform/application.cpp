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

void application_t::handle_event(const any_event_t& event) {
    for (auto* listener : listeners_) {
        switch (event.kind) {
            case event_kind::App:
                listener->onAppEvent(event.data.app);
                break;
            case event_kind::Key:
                listener->onKeyEvent(event.data.key);
                break;
            case event_kind::Text:
                listener->on_text_event(event.text);
                break;
            case event_kind::Mouse:
                listener->onMouseEvent(event.data.mouse);
                break;
            case event_kind::Touch:
                listener->onTouchEvent(event.data.touch);
                break;
        }
    }
}

void application_t::dispatch_draw_frame() {
    if (g_window.size_changed) {
        g_window.size_changed = false;
        handle_event(any_event_t{app_event_t{app_event_type::resize}});
    }
    for (const auto& event : event_queue_) {
        handle_event(event);
    }
    event_queue_.clear();

    for (auto* listener : listeners_) {
        listener->onDrawFrame();
    }

    for (auto* listener : listeners_) {
        listener->on_frame_completed();
    }
}

}