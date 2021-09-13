#include <ek/app/app.hpp>
#include <ek/debug.hpp>
#include <ek/Arguments.hpp>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <unordered_map>

// implementation
#include "impl/analytics_web.h"
#include "impl/device_web.h"
#include "impl/resources_web.h"
#include "impl/sharing_web.h"
#include "impl/user_prefs_web.h"

extern "C" {
extern void web_set_mouse_cursor(int cur);
extern void web_update_gameview_size(double width, double height, double dpr, double offsetX, double offsetY);
}

using namespace ek::app;

void update_mouse_cursor() {
    if (g_app.dirtyCursor) {
        g_app.dirtyCursor = false;
        web_set_mouse_cursor(static_cast<int>(g_app.cursor));
    }
}

void loop() {
    processFrame();
    update_mouse_cursor();
    if (g_app.exitRequired) {
        EM_ASM(window.close());
    }
}

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context;

const char* DIV_ID = "#gamecontainer";
const char* CANVAS_ID = "#gameview";

void init_webgl_context(bool needDepth) {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = false;
    attrs.depth = needDepth;
    attrs.stencil = false;
    attrs.antialias = false;

    webgl_context = emscripten_webgl_create_context(CANVAS_ID, &attrs);

    if (!webgl_context) {
        EK_ERROR("Failed to create WebGL context");
        return;
    }

    EMSCRIPTEN_RESULT result = emscripten_webgl_make_context_current(webgl_context);
    if (result != EMSCRIPTEN_RESULT_SUCCESS) {
        EK_ERROR("Failed to make WebGL context current");
        return;
    }
}

MouseButton toMouseButton(unsigned short btn) {
    switch (btn) {
        case 0:
            return MouseButton::Left;
        case 2:
            return MouseButton::Right;
        default:
            break;
    }
    return MouseButton::Other;
}

static EM_BOOL em_mouse_callback(int type, const EmscriptenMouseEvent* event, void*) {
    MouseEvent mouseEvent{};
    mouseEvent.button = toMouseButton(event->button);
    EventType eventType;

    switch (type) {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
            eventType = EventType::MouseDown;
            break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
            eventType = EventType::MouseUp;
            break;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            eventType = EventType::MouseMove;
            break;
        default:
            return EM_FALSE;
    }

    const auto dpr = g_app.dpiScale;
    mouseEvent.x = dpr * static_cast<float>(event->targetX);
    mouseEvent.y = dpr * static_cast<float>(event->targetY);
    processEvent(Event::Mouse(eventType, mouseEvent));
    return EM_TRUE;
}

/****** KEYBOARD ****/
static std::unordered_map<std::string, KeyCode> scancodeTable = {
        {"ArrowLeft",  KeyCode::ArrowLeft},
        {"ArrowRight", KeyCode::ArrowRight},
        {"ArrowDown",  KeyCode::ArrowDown},
        {"ArrowUp",    KeyCode::ArrowUp},
        {"Escape",     KeyCode::Escape},
        {"Backspace",  KeyCode::Backspace},
        {"Space",      KeyCode::Space},
        {"Enter",      KeyCode::Enter},
        {"A",          KeyCode::A},
        {"C",          KeyCode::C},
        {"V",          KeyCode::V},
        {"X",          KeyCode::X},
        {"Y",          KeyCode::Y},
        {"Z",          KeyCode::Z},
        {"W",          KeyCode::W},
        {"S",          KeyCode::S},
        {"D",          KeyCode::D}
};

static KeyCode toKeyCode(const EM_UTF8 key[EM_HTML5_SHORT_STRING_LEN_BYTES]) {
    const auto i = scancodeTable.find(key);
    return i != scancodeTable.end() ? i->second : KeyCode::Unknown;
}

static EM_BOOL em_keyboard_callback(int type, const EmscriptenKeyboardEvent* event, void*) {
    const KeyEvent keyEvent{
            toKeyCode(event->code),
            KeyModifier::Empty
    };
    EventType eventType;
    switch (type) {
        case EMSCRIPTEN_EVENT_KEYPRESS:
            eventType = EventType::KeyPress_;
            break;
        case EMSCRIPTEN_EVENT_KEYDOWN:
            eventType = EventType::KeyDown;
            break;
        case EMSCRIPTEN_EVENT_KEYUP:
            eventType = EventType::KeyUp;
            break;
        default:
            return EM_FALSE;
    }

    processEvent(Event::Key(eventType, keyEvent));
    return EM_TRUE;
}

static EM_BOOL em_wheel_callback(int type, const EmscriptenWheelEvent* event, void*) {
    if (type == EMSCRIPTEN_EVENT_WHEEL) {
        const auto dpr = g_app.dpiScale;
        MouseEvent mouseEvent{};
        mouseEvent.button = MouseButton::Other;
        mouseEvent.x = dpr * static_cast<float>(event->mouse.targetX);
        mouseEvent.y = dpr * static_cast<float>(event->mouse.targetY);
        mouseEvent.scrollX = static_cast<float>(event->deltaX);
        mouseEvent.scrollY = static_cast<float>(event->deltaY);
        processEvent(Event::Mouse(EventType::MouseScroll, mouseEvent));
    }
    return EM_TRUE;
}


static EM_BOOL em_touch_callback(int type, const EmscriptenTouchEvent* event, void*) {
    const float dpr = g_app.dpiScale;
    for (int i = 0; i < event->numTouches; ++i) {
        const EmscriptenTouchPoint& touch = event->touches[i];
        if (touch.isChanged) {
            TouchEvent touchEvent{};
            touchEvent.id = static_cast<uint64_t>(touch.identifier) + 1;
            touchEvent.x = dpr * static_cast<float>(touch.targetX);
            touchEvent.y = dpr * static_cast<float>(touch.targetY);
            EventType eventType;
            switch (type) {
                case EMSCRIPTEN_EVENT_TOUCHSTART:
                    eventType = EventType::TouchStart;
                    break;
                case EMSCRIPTEN_EVENT_TOUCHMOVE:
                    eventType = EventType::TouchMove;
                    break;
                case EMSCRIPTEN_EVENT_TOUCHEND:
                case EMSCRIPTEN_EVENT_TOUCHCANCEL:
                    eventType = EventType::TouchEnd;
                    break;
                default:
                    continue;
            }
            processEvent(Event::Touch(eventType, touchEvent));
        }
    }

    return EM_TRUE;
}

void subscribe_input() {
    emscripten_set_mousedown_callback(CANVAS_ID, nullptr, false, em_mouse_callback);
    emscripten_set_mouseup_callback(CANVAS_ID, nullptr, false, em_mouse_callback);
    emscripten_set_mousemove_callback(CANVAS_ID, nullptr, false, em_mouse_callback);
    emscripten_set_wheel_callback(CANVAS_ID, nullptr, false, em_wheel_callback);
//    emscripten_set_pointerlockchange_callback(nullptr, nullptr, true, em_pointer_lock_change_callback);

    emscripten_set_touchstart_callback(CANVAS_ID, nullptr, false, em_touch_callback);
    emscripten_set_touchend_callback(CANVAS_ID, nullptr, false, em_touch_callback);
    emscripten_set_touchmove_callback(CANVAS_ID, nullptr, false, em_touch_callback);
    emscripten_set_touchcancel_callback(CANVAS_ID, nullptr, false, em_touch_callback);

    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, em_keyboard_callback);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, em_keyboard_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, em_keyboard_callback);
}

void handle_resize() {
    const auto dpr = emscripten_get_device_pixel_ratio();

    double css_w;
    double css_h;
    emscripten_get_element_css_size(DIV_ID, &css_w, &css_h);

    // TODO: configurable min aspect (70/100)
    // TODO: landscape and different modes, native letterbox
    double w = css_w;
    double h = css_h;
    double offset_x = 0;
    double offset_y = 0;
    if (g_app.config.webKeepCanvasAspectRatio) {
        const double aspect = g_app.config.width / g_app.config.height;
        if (aspect > 1.0) {
            if (w / aspect < h) {
                h = w / aspect;
            }
            offset_y = (css_h - h) / 2;
        } else {
            if (h * aspect < w) {
                w = h * aspect;
            }
            offset_x = (css_w - w) / 2;
        }
    }

    css_w = w;
    css_h = h;

    if (g_app.dpiScale != dpr ||
        g_app.windowWidth != w ||
        g_app.windowHeight != h ||
        g_app.drawableWidth != w * dpr ||
        g_app.drawableHeight != h * dpr) {
        g_app.dirtySize = true;

        g_app.dpiScale = dpr;
        g_app.windowWidth = w;
        g_app.windowHeight = h;
        g_app.drawableWidth = w * dpr;
        g_app.drawableHeight = h * dpr;
    }

    web_update_gameview_size(w, h, dpr, offset_x, offset_y);
}

static EM_BOOL onEmscriptenResize(int type, const EmscriptenUiEvent*, void*) {
    if (type == EMSCRIPTEN_EVENT_RESIZE) {
        handle_resize();
        return EM_TRUE;
    }

    return EM_FALSE;
}

void init_canvas() {
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, onEmscriptenResize);
    handle_resize();
}

namespace ek::app {

void start() {
    init_canvas();
    subscribe_input();
    init_webgl_context(g_app.config.needDepth);

    notifyInit();
    notifyReady();

    emscripten_set_main_loop(&loop, 0, 1);

    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1); // vsync
    //emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 1); // no-vsync?
}

}

int main(int argc, char* argv[]) {
    ::ek::Arguments::current = {argc, argv};
    ::ek::app::main();
    return 0;
}