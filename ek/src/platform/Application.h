#pragma once

#include "window_creation_config.h"

#include <cstdint>
#include <vector>
#include <platform/user_preferences.hpp>

namespace ek {

struct AppEvent {
    enum class Type {
        Paused,
        Resumed,
        Resize,
        BackButton,
        Close
    };
    Type type;
};

struct TouchEvent {
    enum class Type {
        None,
        Begin,
        Move,
        End
    };
    Type type;
    uint64_t id;
    float x;
    float y;
};

struct MouseEvent {
    enum class Type {
        Move,
        Down,
        Up,
        Enter,
        Exit,
        Scroll
    };
    enum class Button {
        Left,
        Right,
        Other
    };
    Type type;
    Button button;
    float x;
    float y;
    float scrollX;
    float scrollY;
};

struct KeyEvent {
    enum class Type {
        Down,
        Up,
        Press
    };
    enum class Code {
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
    Type type;
    Code code;
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
        AppEvent app;
        MouseEvent mouse;
        TouchEvent touch;
        KeyEvent key;
    } data;

    text_event_t text;

    explicit any_event_t(const text_event_t& event) noexcept
            : kind{event_kind::Text},
              data{} {
        text = event;
    }

    explicit any_event_t(const KeyEvent& event) noexcept
            : kind{event_kind::Key},
              data{} {
        data.key = event;
    }

    explicit any_event_t(const AppEvent& event) noexcept
            : kind{event_kind::App},
              data{} {
        data.app = event;
    }

    explicit any_event_t(const MouseEvent& event) noexcept
            : kind{event_kind::Mouse},
              data{} {
        data.mouse = event;
    }

    explicit any_event_t(const TouchEvent& event) noexcept
            : kind{event_kind::Touch},
              data{} {
        data.touch = event;
    }
};

class Application final {
public:

    class Listener {
    public:

        virtual ~Listener();

        virtual void onKeyEvent(const KeyEvent& event) = 0;

        virtual void on_text_event(const text_event_t&) {}

        virtual void onMouseEvent(const MouseEvent& event) = 0;

        virtual void onTouchEvent(const TouchEvent& event) = 0;

        virtual void onAppEvent(const AppEvent& event) = 0;

        virtual void onDrawFrame() = 0;

        virtual void on_frame_completed() {}
    };

    void exit(int code);

    void init();

    void start();

    void listen(Listener* listener) {
        mListeners.push_back(listener);
    }

    void dispatch(const text_event_t& event) {
        event_queue_.emplace_back(event);
    }

    void dispatch(const KeyEvent& event) {
        event_queue_.emplace_back(event);
    }

    void dispatch(const MouseEvent& event) {
        event_queue_.emplace_back(event);
    }

    void dispatch(const TouchEvent& event) {
        event_queue_.emplace_back(event);
    }

    void dispatch(const AppEvent& event) {
        if (event.type == AppEvent::Type::Paused) {
            handle_event(any_event_t{event});
        } else {
            event_queue_.emplace_back(event);
        }
    }

    void handle_event(const any_event_t& event);

    void dispatchDrawFrame();

private:
    std::vector<Listener*> mListeners;

    std::vector<any_event_t> event_queue_;
};

extern Application gApp;

std::string get_device_lang();

void setup_application_window(window_creation_config_t config);
void start_application();

}

void ek_main();