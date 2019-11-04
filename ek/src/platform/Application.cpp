#include <utility>

#include "Application.h"

#include "Window.h"
#include "analytics.hpp"
#include "crash_reporter.hpp"

namespace ek {

Application gApp{};

void setup_application_window(window_creation_config_t config) {
    gWindow.creation_config = std::move(config);
}

Application::Listener::~Listener() = default;

void Application::init() {
    analytics::init(); // analytics before crash reporter on ios
    init_crash_reporter();
}

void Application::start() {
    ek_main();
}

void Application::handle_event(const any_event_t& event) {
    for (auto* listener : mListeners) {
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

void Application::dispatchDrawFrame() {
    if (gWindow.sizeChanged) {
        gWindow.sizeChanged = false;
        handle_event(any_event_t{AppEvent{AppEvent::Type::Resize}});
    }
    for (const auto& event : event_queue_) {
        handle_event(event);
    }
    event_queue_.clear();

    for (auto* listener : mListeners) {
        listener->onDrawFrame();
    }

    for (auto* listener : mListeners) {
        listener->on_frame_completed();
    }
}

}