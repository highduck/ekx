#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <string>
#include <ek/util/signals.hpp>
#include <ek/math/vec.hpp>

namespace ek {

namespace app {

struct vec2 final {
    double x = 0;
    double y = 0;
};

inline vec2 operator*(vec2 v, double s) {
    return {v.x * s, v.y * s};
}

struct window_config final {
    std::string title{"ek"};
    vec2 size{960, 720};
};

struct arguments final {
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


enum class mouse_cursor : uint8_t {
    parent = 0,
    arrow,
    button,
    help
};

enum class event_type : uint8_t {
    app_resume,
    app_pause,
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

    max_count
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
};

struct app_state final {
    arguments args;
    window_config window_cfg;

    vec2 window_size{};
    vec2 drawable_size{};
    double content_scale = 1.0;
    bool size_changed = false;

    // iOS has manually created FBO for primary surface
    // so it will be set up there
    uint32_t primary_frame_buffer = 0;
    void* view_context_ = nullptr;

    bool require_exit = false;
    int exit_code = 0;

    std::string lang;

    mouse_cursor cursor = mouse_cursor::parent;
    bool cursor_dirty = false;

    signal_t<> on_device_ready;
    signal_t<const event_t&> on_event;
    signal_t<> on_frame_draw;
    signal_t<> on_frame_completed;

    std::vector<event_t> event_queue_;
};

extern app_state g_app;

void dispatch_event(const event_t& event);

void dispatch_draw_frame();

void dispatch_init();

void dispatch_device_ready();

}

void start_application();

#ifndef EK_NO_MAIN

void main();

#endif

}