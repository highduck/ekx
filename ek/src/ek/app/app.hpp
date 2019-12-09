#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <string>

namespace ek {

namespace app {

struct vec2 {
    double x, y;
};

struct window_config {
    std::string title{"ek"};
    vec2 size{960, 720};
    bool landscape = false;
    bool fullscreen = false;

    window_config() = default;

    window_config(std::string title_, vec2 size_) noexcept
            : title{std::move(title_)},
              size{size_} {
        landscape = size.x >= size.y;
        fullscreen = true;
    }
};

struct arguments {
    int argc = 0;
    char** argv = nullptr;

    [[nodiscard]]
    std::vector<std::string> to_vector() const {
        using namespace std;
        vector<string> result{};
        result.reserve(argc);
        for (int i = 0; i < argc; ++i) {
            result.emplace_back(argv[i]);
        }
        return result;
    }
};

}

enum class mouse_cursor : uint8_t {
    parent = 0,
    arrow,
    button,
    help
};

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
    unknown = 0,

    ArrowUp,
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

struct event_t final {
    event_type type;
    // touch-id / pointer-id
    uint64_t id = 0;
    app::vec2 pos{0.0, 0.0};
    app::vec2 scroll{0.0, 0.0};
    mouse_button button = mouse_button::other;

    key_code code = key_code::unknown;
    bool super = false; // cmd/win/..
    bool shift = false;
    bool ctrl = false;
    bool alt = false;

    // TODO: view to internal storage
    std::string characters{};

    template<typename T>
    void set_scroll(T x_, T y_, T multiplier_ = 1) {
        scroll.x = static_cast<double>(x_ * multiplier_);
        scroll.y = static_cast<double>(y_ * multiplier_);
    }

    template<typename T>
    void set_position(T x_, T y_, T multiplier_ = 1) {
        pos.x = static_cast<float>(x_ * multiplier_);
        pos.y = static_cast<float>(y_ * multiplier_);
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
    app::arguments args;
    app::vec2 window_size{};
    app::vec2 drawable_size{};
    double content_scale{1.0};

    // iOS has manually created FBO for primary surface
    // so it will be set up there
    uint32_t primary_frame_buffer = 0;

    bool size_changed = false;

    app::window_config window_config;

    void* view_context_ = nullptr;

    bool require_exit = false;
    int exit_code = 0;

    std::string lang;

    mouse_cursor cursor = mouse_cursor::parent;
    bool cursor_dirty = false;

    std::vector<application_listener_t*> listeners_;
    std::vector<event_t> event_queue_;

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
};

extern application_t g_app;

void dispatch_init();

void dispatch_device_ready();

void start_application();

#ifndef EK_NO_MAIN

void main();

#endif

}

void ek_main();