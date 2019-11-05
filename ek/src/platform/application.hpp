#pragma once

#include <platform/window.hpp>
#include <platform/user_preferences.hpp>

#include <cstdint>
#include <vector>

namespace ek {

enum class app_event_type : uint8_t {
    paused,
    resumed,
    resize,
    back_button,
    close
};

struct app_event_t {
    app_event_type type;
};

enum class touch_event_type : uint8_t {
    none,
    begin,
    move,
    end
};

struct touch_event_t {
    touch_event_type type;
    uint64_t id;
    float x;
    float y;
};

enum class mouse_event_type {
    move,
    down,
    up,
    enter,
    exit,
    scroll
};

enum class mouse_button {
    left,
    right,
    other
};

struct mouse_event_t {
    mouse_event_type type;
    mouse_button button;
    float x;
    float y;
    float scroll_x;
    float scroll_y;
};

enum class key_event_type {
    down,
    up,
    press
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

struct key_event_t {
    key_event_type type;
    key_code code;
    bool super = false; // cmd/win/..
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};

struct text_event_t {
    std::string characters;
};

enum class event_kind {
    App,
    Key,
    Mouse,
    Touch,
    Text
};

struct any_event_t {
    event_kind kind;
    union data_t {
        app_event_t app;
        mouse_event_t mouse;
        touch_event_t touch;
        key_event_t key;
    } data;

    text_event_t text;

    explicit any_event_t(const text_event_t& event) noexcept
            : kind{event_kind::Text},
              data{} {
        text = event;
    }

    explicit any_event_t(const key_event_t& event) noexcept
            : kind{event_kind::Key},
              data{} {
        data.key = event;
    }

    explicit any_event_t(const app_event_t& event) noexcept
            : kind{event_kind::App},
              data{} {
        data.app = event;
    }

    explicit any_event_t(const mouse_event_t& event) noexcept
            : kind{event_kind::Mouse},
              data{} {
        data.mouse = event;
    }

    explicit any_event_t(const touch_event_t& event) noexcept
            : kind{event_kind::Touch},
              data{} {
        data.touch = event;
    }
};

class application_listener_t {
public:

    virtual ~application_listener_t();

    virtual void onKeyEvent(const key_event_t& event) = 0;

    virtual void on_text_event(const text_event_t&) {}

    virtual void onMouseEvent(const mouse_event_t& event) = 0;

    virtual void onTouchEvent(const touch_event_t& event) = 0;

    virtual void onAppEvent(const app_event_t& event) = 0;

    virtual void onDrawFrame() = 0;

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

    void dispatch(const text_event_t& event) {
        event_queue_.emplace_back(event);
    }

    void dispatch(const key_event_t& event) {
        event_queue_.emplace_back(event);
    }

    void dispatch(const mouse_event_t& event) {
        event_queue_.emplace_back(event);
    }

    void dispatch(const touch_event_t& event) {
        event_queue_.emplace_back(event);
    }

    void dispatch(const app_event_t& event) {
        if (event.type == app_event_type::paused) {
            handle_event(any_event_t{event});
        } else {
            event_queue_.emplace_back(event);
        }
    }

    void handle_event(const any_event_t& event);

    void dispatch_draw_frame();

private:
    std::vector<application_listener_t*> listeners_;

    std::vector<any_event_t> event_queue_;
};

extern application_t g_app;

std::string get_device_lang();

void setup_application_window(window_creation_config_t config);

void start_application();

}

void ek_main();