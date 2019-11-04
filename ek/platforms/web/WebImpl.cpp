#include <ek/logger.hpp>
#include "platform/Application.h"
#include "platform/Window.h"
#include "platform/static_resources.hpp"
#include "platform/Sharing.h"

#include <emscripten.h>
#include <emscripten/html5.h>

#include <unordered_map>
#include <string>

using namespace ek;

void loop() {
    gApp.dispatchDrawFrame();
}

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webGLContext;

const char* DIV_ID = "gamecontainer";
const char* CANVAS_ID = "gameview";

void init_webgl_context() {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = false;
    attrs.depth = false;
    attrs.stencil = false;
    attrs.antialias = false;

    webGLContext = emscripten_webgl_create_context(CANVAS_ID, &attrs);

    if (!webGLContext) {
        assert(false && "Failed to create WebGL context");
        return;
    }

    EMSCRIPTEN_RESULT result = emscripten_webgl_make_context_current(webGLContext);
    if (result != EMSCRIPTEN_RESULT_SUCCESS) {
        assert(false && "Failed to make WebGL context current");
        return;
    }
}

static EM_BOOL em_mouse_callback(int type, const EmscriptenMouseEvent* mouse_event, void*) {
    MouseEvent event{};

    switch (mouse_event->button) {
        case 0:
            event.button = MouseEvent::Button::Left;
            break;
        case 1:
            event.button = MouseEvent::Button::Other;
            break;
        case 2:
            event.button = MouseEvent::Button::Right;
            break;
        default:
//            return false;
            break;
    }

    switch (type) {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
            event.type = MouseEvent::Type::Down;
            break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
            event.type = MouseEvent::Type::Up;
            break;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            event.type = MouseEvent::Type::Move;
            break;
        default:
            // return false;
            break;
    }

    event.x = static_cast<float>(mouse_event->targetX * gWindow.scaleFactor);
    event.y = static_cast<float>(mouse_event->targetY * gWindow.scaleFactor);
    gApp.dispatch(event);

    return true;
}

/****** KEYBOARD ****/
static std::unordered_map<std::string, KeyEvent::Code> scancode_table = {
        {"ArrowLeft", KeyEvent::Code::ArrowLeft},
        {"ArrowRight", KeyEvent::Code::ArrowRight},
        {"ArrowDown", KeyEvent::Code::ArrowDown},
        {"ArrowUp", KeyEvent::Code::ArrowUp},
        {"Escape", KeyEvent::Code::Escape},
        {"Backspace", KeyEvent::Code::Backspace},
        {"Space", KeyEvent::Code::Space},
        {"Enter", KeyEvent::Code::Enter},
        {"A", KeyEvent::Code::A},
        {"C", KeyEvent::Code::C},
        {"V", KeyEvent::Code::V},
        {"X", KeyEvent::Code::X},
        {"Y", KeyEvent::Code::Y},
        {"Z", KeyEvent::Code::Z},
        {"W", KeyEvent::Code::W},
        {"S", KeyEvent::Code::S},
        {"D", KeyEvent::Code::D}
};

static KeyEvent::Code convertKeyCode(const EM_UTF8 keyCode[EM_HTML5_SHORT_STRING_LEN_BYTES]) {
    auto i = scancode_table.find(keyCode);

    if (i != scancode_table.end())
        return i->second;
    else
        return KeyEvent::Code::Unknown;
}

static EM_BOOL web_onKeyboardEvent(int type, const EmscriptenKeyboardEvent* event, void*) {
    switch (type) {
        case EMSCRIPTEN_EVENT_KEYPRESS:
            gApp.dispatch({KeyEvent::Type::Press, convertKeyCode(event->code)});
            return true;
        case EMSCRIPTEN_EVENT_KEYDOWN:
            gApp.dispatch({KeyEvent::Type::Down, convertKeyCode(event->code)});
            return true;
        case EMSCRIPTEN_EVENT_KEYUP:
            gApp.dispatch({KeyEvent::Type::Up, convertKeyCode(event->code)});
            return true;
    }

    return false;
}

static EM_BOOL em_wheel_callback(int type, const EmscriptenWheelEvent* event, void*) {
    if (type == EMSCRIPTEN_EVENT_WHEEL) {
        float x = event->mouse.targetX * gWindow.scaleFactor;
        float y = event->mouse.targetY * gWindow.scaleFactor;
        auto sx = static_cast<float>(event->deltaX);
        auto sy = static_cast<float>(event->deltaY);
        gApp.dispatch({MouseEvent::Type::Scroll, MouseEvent::Button::Other, x, y, sx, sy});

        //    return true;
    }

//    return false;
    return true;
}


static EM_BOOL handleTouchEvent(int type, const EmscriptenTouchEvent* event, void*) {
    for (int i = 0; i < event->numTouches; ++i) {
        const EmscriptenTouchPoint& touch = event->touches[i];

        if (touch.isChanged) {
            auto x = static_cast<float>(touch.targetX * gWindow.scaleFactor);
            auto y = static_cast<float>(touch.targetY * gWindow.scaleFactor);
            // zero for unknown
            auto id = static_cast<uint64_t>(touch.identifier + 1);
            switch (type) {
                case EMSCRIPTEN_EVENT_TOUCHSTART:
                    gApp.dispatch({TouchEvent::Type::Begin, id, x, y});
                    break;
                case EMSCRIPTEN_EVENT_TOUCHMOVE:
                    gApp.dispatch({TouchEvent::Type::Move, id, x, y});
                    break;
                case EMSCRIPTEN_EVENT_TOUCHEND:
                    gApp.dispatch({TouchEvent::Type::End, id, x, y});
                    break;
                case EMSCRIPTEN_EVENT_TOUCHCANCEL:
                    gApp.dispatch({TouchEvent::Type::End, id, x, y});
                    break;
                default:
                    break;
            }
        }
    }

    return true;
}

void subscribeInput() {
    emscripten_set_mousedown_callback(CANVAS_ID, nullptr, false, em_mouse_callback);
    emscripten_set_mouseup_callback(CANVAS_ID, nullptr, false, em_mouse_callback);
    emscripten_set_mousemove_callback(CANVAS_ID, nullptr, false, em_mouse_callback);
    emscripten_set_wheel_callback(CANVAS_ID, nullptr, false, em_wheel_callback);
//    emscripten_set_pointerlockchange_callback(nullptr, nullptr, true, emPointerLockChangeCallback);

    emscripten_set_touchstart_callback(CANVAS_ID, nullptr, false, handleTouchEvent);
    emscripten_set_touchend_callback(CANVAS_ID, nullptr, false, handleTouchEvent);
    emscripten_set_touchmove_callback(CANVAS_ID, nullptr, false, handleTouchEvent);
    emscripten_set_touchcancel_callback(CANVAS_ID, nullptr, false, handleTouchEvent);

    emscripten_set_keypress_callback(nullptr, nullptr, true, web_onKeyboardEvent);
    emscripten_set_keydown_callback(nullptr, nullptr, true, web_onKeyboardEvent);
    emscripten_set_keyup_callback(nullptr, nullptr, true, web_onKeyboardEvent);
}

void handleResize() {
    float scaleFactor = emscripten_get_device_pixel_ratio();

    double css_w, css_h;
    emscripten_get_element_css_size(DIV_ID, &css_w, &css_h);

    // TODO: configurable min aspect (70/100)
    // TODO: landscape and different modes, native letterbox
    float aspect = gWindow.creation_config.width / gWindow.creation_config.height;
    int w = css_w;
    int h = css_h;
    int offset_x = 0;
    int offset_y = 0;
    if (gWindow.creation_config.landscape) {
        h = std::min(h, static_cast<int>(w / aspect));
        offset_y = (css_h - h) / 2;
    } else {
        w = std::min(w, static_cast<int>(h * aspect));
        offset_x = (css_w - w) / 2;
    };

    css_w = w;
    css_h = h;

    gWindow.scaleFactor = scaleFactor;

    gWindow.windowSize = {
            static_cast<uint32_t>(css_w),
            static_cast<uint32_t>(css_h)
    };

    gWindow.backBufferSize = {
            static_cast<uint32_t>(css_w * scaleFactor),
            static_cast<uint32_t>(css_h * scaleFactor)
    };

    emscripten_set_canvas_element_size(CANVAS_ID,
                                       static_cast<int>(gWindow.backBufferSize.width),
                                       static_cast<int>(gWindow.backBufferSize.height));

    emscripten_set_element_css_size(CANVAS_ID, css_w, css_h);

    EM_ASM({document.getElementById("gameview").style["transform"] =
                    "translateX(" + $0 + "px) translateY(" + $1 + "px)";}, offset_x, offset_y);
}

static EM_BOOL onEmscriptenResize(int type, const EmscriptenUiEvent*, void*) {
    if (type == EMSCRIPTEN_EVENT_RESIZE) {
        handleResize();
        return true;
    }

    return false;
}

void init_canvas() {
    emscripten_set_resize_callback(nullptr, nullptr, true, onEmscriptenResize);
    handleResize();
}

EM_JS(void, emscripten_set_canvas_cursor,(const char* type),

{
document.getElementById("gameview").style["cursor"] =
UTF8ToString(type);
});

namespace ek {

void start_application() {
    init_canvas();
    subscribeInput();
    init_webgl_context();

    gApp.init();
    gApp.start();

    emscripten_set_main_loop(&loop, 0, 1);

    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1); // vsync
    //emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 1); // no-vsync?
}

void Window::updateMouseCursor() {
    const char* cursorName = "default";
    switch (mCursor) {
        case Window::Cursor::Button:
            cursorName = "pointer";
            break;
        case Window::Cursor::Help:
            cursorName = "help";
            break;
        default:
            cursorName = "default";
            break;
    }

    emscripten_set_canvas_cursor(cursorName);
}

void Application::exit(int) {
    EM_ASM({window.close()}, 0);
}

}


// SHARING
namespace ek {
void sharing_navigate(const char* url) {
    EM_ASM({window.open(UTF8ToString($0), "_blank")}, url);
}

void sharing_rate_us(const char* appId) {
    (void)appId;
}

void sharing_send_message(const char* text) {
    (void)text;
}


}