#pragma once

#include <platform/window.hpp>
#include <platform/user_preferences.hpp>

#include <cstdint>
#include <vector>

namespace ek {

enum class event_type : uint8_t {
    app_pause,
    app_resume,
    app_resize,
    app_back_button,
    app_close,

    touch_begin,
    touch_move,
    touch_end,

    mouse_move,
    mouse_down,
    mouse_up,
    mouse_enter,
    mouse_exit,
    mouse_scroll,

    key_down,
    key_up,
    key_press,

    text
};

enum class mouse_button {
    left,
    right,
    other
};

enum class key_code {
    Unknown = -1,

    ArrowUp = 0,
    ArrowDown,
    ArrowLeft,
    ArrowRight,

    Escape,
    Space,
    Enter,
    Backspace,

    // extra
            Tab,
    PageUp,
    PageDown,
    Home,
    End,
    Insert,
    Delete,

    A,
    C,
    V,
    X,
    Y,
    Z,

    W,
    S,
    D,

    MaxCount
};


struct event_t {
    event_type type;
    // touch_id, or mouse_button
    uint64_t id = 0;
    float x = 0.0f;
    float y = 0.0f;
    float scroll_x = 0.0f;
    float scroll_y = 0.0f;
    mouse_button button = mouse_button::other;

    key_code code = key_code::Unknown;
    bool super = false; // cmd/win/..
    bool shift = false;
    bool ctrl = false;
    bool alt = false;

    // TODO: view to internal storage
    std::string characters{};

    template<typename T>
    void set_mouse_scroll(T x_, T y_, T multiplier_ = 1) {
        scroll_x = static_cast<float>(x_ * multiplier_);
        scroll_y = static_cast<float>(y_ * multiplier_);
    }

    template<typename T>
    void set_position(T x_, T y_, T multiplier_ = 1) {
        x = static_cast<float>(x_ * multiplier_);
        y = static_cast<float>(y_ * multiplier_);
    }
};

class application_listener_t {
public:

    virtual ~application_listener_t();

    virtual void on_event(const event_t&) {}

    virtual void on_draw_frame() {}

    virtual void on_frame_completed() {}
};

class application_t final {
public:

    void exit(int code);

    void init();

    void start();

    void listen(application_listener_t* listener) {
        listeners_.push_back(listener);
    }

    void dispatch(const event_t& event) {
        if (event.type == event_type::app_pause) {
            handle_event(event);
        } else {
            event_queue_.emplace_back(event);
        }
    }

    void handle_event(const event_t& event);

    void dispatch_draw_frame();

private:
    std::vector<application_listener_t*> listeners_;

    std::vector<event_t> event_queue_;
};

extern application_t g_app;

std::string get_device_lang();

void setup_application_window(window_creation_config_t config);

void start_application();

}

void ek_main();