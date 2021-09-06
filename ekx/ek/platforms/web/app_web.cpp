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
    if (g_app.cursor_dirty) {
        g_app.cursor_dirty = false;
        web_set_mouse_cursor(static_cast<int>(g_app.cursor));
    }
}

void loop() {
    dispatch_draw_frame();
    update_mouse_cursor();
    if (g_app.require_exit) {
        EM_ASM({window.close()}, 0);
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

static EM_BOOL em_mouse_callback(int type, const EmscriptenMouseEvent* mouseEvent, void*) {
    Event event{};
    event.button = toMouseButton(mouseEvent->button);

    switch (type) {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
            event.type = Event::MouseDown;
            break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
            event.type = Event::MouseUp;
            break;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            event.type = Event::MouseMove;
            break;
        default:
            return EM_FALSE;
    }

    const auto dpr = g_app.content_scale;
    event.pos.x = dpr * static_cast<float>(mouseEvent->targetX);
    event.pos.y = dpr * static_cast<float>(mouseEvent->targetY);
    dispatch_event(event);
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
    Event ev{};
    switch (type) {
        case EMSCRIPTEN_EVENT_KEYPRESS:
            ev.type = Event::KeyPress_;
            ev.keyCode = toKeyCode(event->code);
            dispatch_event(ev);
            return EM_TRUE;
        case EMSCRIPTEN_EVENT_KEYDOWN:
            ev.type = Event::KeyDown;
            ev.keyCode = toKeyCode(event->code);
            dispatch_event(ev);
            return EM_TRUE;
        case EMSCRIPTEN_EVENT_KEYUP:
            ev.type = Event::KeyUp;
            ev.keyCode = toKeyCode(event->code);
            dispatch_event(ev);
            return EM_TRUE;
        default:
            break;
    }

    return EM_FALSE;
}

static EM_BOOL em_wheel_callback(int type, const EmscriptenWheelEvent* event, void*) {
    if (type == EMSCRIPTEN_EVENT_WHEEL) {
        Event ev{Event::MouseScroll};
        const auto dpr = g_app.content_scale;
        ev.pos.x = dpr * static_cast<float>(event->mouse.targetX);
        ev.pos.y = dpr * static_cast<float>(event->mouse.targetY);
        ev.scroll.x = static_cast<float>(event->deltaX);
        ev.scroll.y = static_cast<float>(event->deltaY);
        dispatch_event(ev);
    }
    return EM_TRUE;
}


static EM_BOOL em_touch_callback(int type, const EmscriptenTouchEvent* event, void*) {
    Event ev{};
    const float dpr = g_app.content_scale;
    for (int i = 0; i < event->numTouches; ++i) {
        const EmscriptenTouchPoint& touch = event->touches[i];
        if (touch.isChanged) {
            ev.id = static_cast<uint64_t>(touch.identifier) + 1;
            ev.pos.x = dpr * static_cast<float>(touch.targetX);
            ev.pos.y = dpr * static_cast<float>(touch.targetY);
            switch (type) {
                case EMSCRIPTEN_EVENT_TOUCHSTART:
                    ev.type = Event::TouchBegin;
                    dispatch_event(ev);
                    break;
                case EMSCRIPTEN_EVENT_TOUCHMOVE:
                    ev.type = Event::TouchMove;
                    dispatch_event(ev);
                    break;
                case EMSCRIPTEN_EVENT_TOUCHEND:
                case EMSCRIPTEN_EVENT_TOUCHCANCEL:
                    ev.type = Event::TouchEnd;
                    dispatch_event(ev);
                    break;
                default:
                    break;
            }
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
    if (g_app.window_cfg.webKeepCanvasAspectRatio) {
        const double aspect = g_app.window_cfg.size.x / g_app.window_cfg.size.y;
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

    if (g_app.content_scale != dpr ||
        g_app.window_size.x != w ||
        g_app.window_size.y != h ||
        g_app.drawable_size.x != w * dpr ||
        g_app.drawable_size.y != h * dpr) {
        g_app.size_changed = true;

        g_app.content_scale = dpr;
        g_app.window_size.x = w;
        g_app.window_size.y = h;
        g_app.drawable_size.x = w * dpr;
        g_app.drawable_size.y = h * dpr;
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

namespace ek {

void start_application() {
    init_canvas();
    subscribe_input();
    init_webgl_context(g_app.window_cfg.needDepth);

    dispatch_init();
    dispatch_device_ready();

    emscripten_set_main_loop(&loop, 0, 1);

    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1); // vsync
    //emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 1); // no-vsync?
}

}

int main(int argc, char* argv[]) {
    ::ek::Arguments::current = {argc, argv};
    ::ek::main();
    return 0;
}