#pragma once

#include <cstdint>
#include <cstring>

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
    EventType type;
    char data[8];

    explicit TextEvent(const char* text) : type{EventType::Text},
                                           data{""} {
        set(text);
    }

    void set(const char* text) {
        data[0] = '\0';
        strncat(data, text, 7);
    }

    [[nodiscard]]
    bool empty() const {
        return data[0] == '\0';
    }
};

struct KeyEvent final {
    EventType type;
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
    EventType type;
    uint64_t id;
    float x;
    float y;
};

struct MouseEvent final {
    EventType type;
    MouseButton button;
    float x;
    float y;
    float scrollX;
    float scrollY;
};

union Event final {
    EventType type;
    KeyEvent key;
    TextEvent text;
    TouchEvent touch;
    MouseEvent mouse;

    Event(EventType eventType) : type{eventType} {
    }

    Event(KeyEvent keyEvent) : key{keyEvent} {
    }

    Event(TouchEvent touchEvent) : touch{touchEvent} {
    }

    Event(MouseEvent mouseEvent) : mouse{mouseEvent} {
    }

    Event(TextEvent textEvent) : text{textEvent} {
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

struct AppConfig final {
    const char* title = nullptr;
    float width = 960;
    float height = 720;
    bool needDepth = false;
    bool webKeepCanvasAspectRatio = false;
    bool allowHighDpi = true;
    int sampleCount = 1;
    int swapInterval = 1;
    uint32_t backgroundColor = 0xFF000000u;
};

struct AppContext {
    AppConfig config;

    float windowWidth = 1.0f;
    float windowHeight = 1.0f;
    float drawableWidth = 1.0f;
    float drawableHeight = 1.0f;
    float dpiScale = 1.0f;

    int exitCode = 0;

    MouseCursor cursor = MouseCursor::Parent;
    AppListener* listener = nullptr;

    bool initialized = false;
    bool ready = false;
    bool running = false;

    bool fullscreen = false;
    bool exitRequired = false;
    bool dirtySize = false;
    bool dirtyCursor = false;
};

inline AppContext g_app{};

void start();

void quit(int status);

void cancelQuit();

void setMouseCursor(MouseCursor cursor_);

/**
 * Default lang on device during application creation (currently default for user on the device)
 * @return pointer to language code string or nullptr in case of error or not supported
 */
const char* getPreferredLang();

/**
 * Safe area margins left-top-right-bottom
 * @return pointer to 4 floats array, nullptr in case if not available
 */
const float* getScreenInsets();

int vibrate(int durationMillis);

int openURL(const char* url);

const char* getSystemFontPath(const char* fontName);

#ifndef EK_NO_MAIN

void main();

#endif

// internal functions
void processEvent(const Event& event);

void processFrame();

void notifyInit();

void notifyReady();

}

#if 1 || defined(EKAPP_DEBUG)

#include <cstdio>
#include <cassert>

#define EKAPP_LOG(s) puts(s)
#define EKAPP_ASSERT(s) assert(s)

#else

#define EKAPP_LOG(s) ((void)(0))
#define EKAPP_ASSERT(s) ((void)(0))

#endif