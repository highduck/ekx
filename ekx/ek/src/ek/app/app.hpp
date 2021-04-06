#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <string>
#include <ek/util/signals.hpp>
#include <ek/util/StaticSignal.hpp>
#include <ek/math/vec.hpp>
#include <ek/ds/Array.hpp>
#include <ek/util/detect_platform.hpp>

namespace ek {

namespace app {

#if EK_MACOS || EK_IOS
void* getMetalDevice();
const void* getMetalRenderPass();
const void* getMetalDrawable();
#endif

struct window_config final {
    std::string title{"ek"};
    float2 size{960, 720};
    bool needDepth = false;
    bool webKeepCanvasAspectRatio = false;
};

struct arguments final {
    int argc = 0;
    char** argv = nullptr;

    [[nodiscard]]
    std::vector<std::string> to_vector() const;
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

    text,

    max_count
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
    float2 pos{0.0f, 0.0f};
    float2 scroll{0.0f, 0.0f};
    mouse_button button = mouse_button::other;

    // TODO: view to internal storage
    std::string characters{};

    key_code code = key_code::unknown;
    bool super = false; // cmd/win/..
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};

struct app_state final {
    arguments args;
    window_config window_cfg;

    bool fallbackGLES2 = false;
    float2 window_size{};
    float2 drawable_size{};

    // TODO: rename to dpiScale (content misunderstood versus game view scaling)
    float content_scale = 1.0f;
    bool size_changed = false;

    // iOS has manually created FBO for primary surface
    // so it will be set up there
    uint32_t primary_frame_buffer = 0;
    void* view_context_ = nullptr;

    bool require_exit = false;
    int exit_code = 0;

    mouse_cursor cursor = mouse_cursor::parent;
    bool cursor_dirty = false;

    StaticSignal<> on_device_ready;
    StaticSignal<const event_t&> on_event;
    StaticSignal<> on_frame_draw;
    StaticSignal<> on_frame_completed;

    Array<event_t> event_queue_;
    Array<event_t> pool_queue;
    bool event_queue_locked = false;

    // we change counter from posted native events queue
    int systemPauseCounter = 0;
    // we decide if system should be paused or resumed
    bool systemPaused = false;

    // as we handle application pause event after `systemPaused`
    // we update `applicationInFocus`
    bool applicationInFocus = true;

    void updateMouseCursor(mouse_cursor cursor_);
};

extern app_state& g_app;

void dispatch_event(const event_t& event);

void dispatch_draw_frame();

void dispatch_init();

void dispatch_device_ready();

void initialize();

void shutdown();

}

void start_application();

#ifndef EK_NO_MAIN

void main();

#endif


}