#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <string>
#include <ek/util/Signal.hpp>
#include <ek/util/StaticSignal.hpp>
#include <ek/math/vec.hpp>
#include <ek/ds/Array.hpp>
#include <ek/util/Platform.hpp>

namespace ek {

namespace app {

#if EK_MACOS || EK_IOS

void* getMetalDevice();

const void* getMetalRenderPass();

const void* getMetalDrawable();

#endif

struct WindowConfig final {
    const char* title = nullptr;
    float2 size{960, 720};
    bool needDepth = false;
    bool webKeepCanvasAspectRatio = false;
    int sampleCount = 1;
    int swapInterval = 1;
    bool allowHighDpi = true;
    uint32_t backgroundColor = 0xFF000000u;
};

enum class MouseCursor {
    Parent = 0,
    Arrow,
    Button,
    Help
};

enum class MouseButton {
    Left,
    Right,
    Other
};

enum class KeyCode {
    Unknown = 0,

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

enum class KeyModifier {
    Empty = 0,
    // Super is "command" or "windows" key
    Super = 1,
    Shift = 2,
    Control = 4,
    Alt = 8
};

struct Event final {
    enum Type {
        Resume = 0,
        // sync version of pause callback, don't do any criminal here :)
        Pause,
        Resize,
        BackButton,
        Close,

        TouchBegin,
        TouchMove,
        TouchEnd,

        MouseMove,
        MouseDown,
        MouseUp,
        MouseEnter,
        MouseExit,
        MouseScroll,

        KeyDown,
        KeyUp,
        // TODO: since keypress deprecated on web, check if we need it
        // TODO: `KeyPress` macro pollution from X11 headers
        KeyPress_,

        Text,

        Count
    };
    Type type;

    // touch-id / pointer-id
    uint64_t id = 0;
    float2 pos{0.0f, 0.0f};
    float2 scroll{0.0f, 0.0f};
    MouseButton button = MouseButton::Other;

    char characters[8] = "";

    KeyCode keyCode = KeyCode::Unknown;
    KeyModifier keyModifiers = KeyModifier::Empty;

    void setCharacters(const char* source);
};

struct app_state final {
    WindowConfig window_cfg;

    float2 window_size{};
    float2 drawable_size{};
    bool fullscreen = false;

    // TODO: rename to dpiScale (content misunderstood versus game view scaling)
    float content_scale = 1.0f;

    bool size_changed = false;
    bool require_exit = false;
    int exit_code = 0;

    MouseCursor cursor = MouseCursor::Parent;
    bool cursor_dirty = false;

    StaticSignal<> on_device_ready;
    StaticSignal<const Event&> on_event;
    StaticSignal<> on_frame_draw;
    StaticSignal<> on_frame_completed;

    Array<Event> event_queue_;
    Array<Event> pool_queue;
    bool event_queue_locked = false;

    // we change counter from posted native events queue
    int systemPauseCounter = 0;
    // we decide if system should be paused or resumed
    bool systemPaused = false;

    // as we handle application pause event after `systemPaused`
    // we update `applicationInFocus`
    bool applicationInFocus = true;

    void updateMouseCursor(MouseCursor cursor_);
};

extern app_state& g_app;

void process_event(const Event& event);

void dispatch_event(const Event& event);

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