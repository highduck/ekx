#pragma once

#include <cstring>
#include <cstdint>

namespace ek::app {

enum EventType {
    Resume = 0,
    Pause,
    Resize,
    BackButton,
    Close,

    TouchStart,
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

struct TextEvent final {
    char data[8];

    void set(const char* source) {
        data[0] = '\0';
        strncat(data, source, 7);
    }

    [[nodiscard]]
    bool empty() const {
        return data[0] != '\0';
    }
};

struct KeyEvent final {
    KeyCode code;
    KeyModifier modifiers;

    [[nodiscard]]
    inline bool isControl() const {
        return ((int) modifiers & (int) KeyModifier::Control) != 0;
    }

    [[nodiscard]]
    inline bool isShift() const {
        return ((int) modifiers & (int) KeyModifier::Shift) != 0;
    }

    [[nodiscard]]
    inline bool isAlt() const {
        return ((int) modifiers & (int) KeyModifier::Alt) != 0;
    }

    [[nodiscard]]
    inline bool isSuper() const {
        return ((int) modifiers & (int) KeyModifier::Super) != 0;
    }
};

struct TouchEvent final {
    uint64_t id;
    float x;
    float y;
};

struct MouseEvent final {
    MouseButton button;
    float x;
    float y;
    float scrollX;
    float scrollY;
};

struct Event final {

    EventType type;

    union {
        KeyEvent key;
        TextEvent text;
        TouchEvent touch;
        MouseEvent mouse;
    };

    static Event App(EventType type) {
        Event ev{};
        ev.type = type;
        return ev;
    }

    static Event Key(EventType type, KeyEvent key) {
        Event ev{};
        ev.type = type;
        ev.key = key;
        return ev;
    }

    static Event Touch(EventType type, TouchEvent touch) {
        Event ev{};
        ev.type = type;
        ev.touch = touch;
        return ev;
    }

    static Event Mouse(EventType type, MouseEvent mouse) {
        Event ev{};
        ev.type = type;
        ev.mouse = mouse;
        return ev;
    }

    static Event TextEvent(const char* data) {
        Event ev{};
        ev.type = EventType::Text;
        ev.text.set(data);
        return ev;
    }
};

class AppListener {
public:
    virtual ~AppListener();

    virtual void onInitialize() {}
    virtual void onReady() {}
    virtual void onFrame() {}

    virtual void onEvent(const Event&) {}
};

inline AppListener::~AppListener() = default;

}
