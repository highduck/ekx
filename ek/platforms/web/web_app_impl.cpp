#include <platform/application.hpp>
#include <platform/static_resources.hpp>
#include <platform/Sharing.h>
#include <ek/logger.hpp>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <unordered_map>
#include <string>

using namespace ek;

void loop() {
    g_app.dispatch_draw_frame();
}

EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context;

const char* DIV_ID = "gamecontainer";
const char* CANVAS_ID = "gameview";

void init_webgl_context() {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = false;
    attrs.depth = false;
    attrs.stencil = false;
    attrs.antialias = false;

    webgl_context = emscripten_webgl_create_context(CANVAS_ID, &attrs);

    if (!webgl_context) {
        assert(false && "Failed to create WebGL context");
        return;
    }

    EMSCRIPTEN_RESULT result = emscripten_webgl_make_context_current(webgl_context);
    if (result != EMSCRIPTEN_RESULT_SUCCESS) {
        assert(false && "Failed to make WebGL context current");
        return;
    }
}

static EM_BOOL em_mouse_callback(int type, const EmscriptenMouseEvent* mouse_event, void*) {
    mouse_event_t event{};

    switch (mouse_event->button) {
        case 0:
            event.button = mouse_button::left;
            break;
        case 1:
            event.button = mouse_button::other;
            break;
        case 2:
            event.button = mouse_button::right;
            break;
        default:
//            return false;
            break;
    }

    switch (type) {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
            event.type = mouse_event_type::down;
            break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
            event.type = mouse_event_type::up;
            break;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            event.type = mouse_event_type::move;
            break;
        default:
            // return false;
            break;
    }

    const auto dpr = g_window.device_pixel_ratio;
    event.x = static_cast<float>(mouse_event->targetX * dpr);
    event.y = static_cast<float>(mouse_event->targetY * dpr);
    g_app.dispatch(event);

    return true;
}

/****** KEYBOARD ****/
static std::unordered_map<std::string, key_code> scancode_table = {
        {"ArrowLeft",  key_code::ArrowLeft},
        {"ArrowRight", key_code::ArrowRight},
        {"ArrowDown",  key_code::ArrowDown},
        {"ArrowUp",    key_code::ArrowUp},
        {"Escape",     key_code::Escape},
        {"Backspace",  key_code::Backspace},
        {"Space",      key_code::Space},
        {"Enter",      key_code::Enter},
        {"A",          key_code::A},
        {"C",          key_code::C},
        {"V",          key_code::V},
        {"X",          key_code::X},
        {"Y",          key_code::Y},
        {"Z",          key_code::Z},
        {"W",          key_code::W},
        {"S",          key_code::S},
        {"D",          key_code::D}
};

static key_code convertKeyCode(const EM_UTF8 keyCode[EM_HTML5_SHORT_STRING_LEN_BYTES]) {
    auto i = scancode_table.find(keyCode);

    if (i != scancode_table.end())
        return i->second;
    else
        return key_code::Unknown;
}

static EM_BOOL web_onKeyboardEvent(int type, const EmscriptenKeyboardEvent* event, void*) {
    switch (type) {
        case EMSCRIPTEN_EVENT_KEYPRESS:
            g_app.dispatch({key_event_type::press, convertKeyCode(event->code)});
            return true;
        case EMSCRIPTEN_EVENT_KEYDOWN:
            g_app.dispatch({key_event_type::down, convertKeyCode(event->code)});
            return true;
        case EMSCRIPTEN_EVENT_KEYUP:
            g_app.dispatch({key_event_type::up, convertKeyCode(event->code)});
            return true;
    }

    return false;
}

static EM_BOOL em_wheel_callback(int type, const EmscriptenWheelEvent* event, void*) {
    if (type == EMSCRIPTEN_EVENT_WHEEL) {
        float x = event->mouse.targetX * g_window.device_pixel_ratio;
        float y = event->mouse.targetY * g_window.device_pixel_ratio;
        auto sx = static_cast<float>(event->deltaX);
        auto sy = static_cast<float>(event->deltaY);
        g_app.dispatch({mouse_event_type::scroll, mouse_button::other, x, y, sx, sy});

        //    return true;
    }

//    return false;
    return true;
}


static EM_BOOL handleTouchEvent(int type, const EmscriptenTouchEvent* event, void*) {
    for (int i = 0; i < event->numTouches; ++i) {
        const EmscriptenTouchPoint& touch = event->touches[i];

        if (touch.isChanged) {
            const auto dpr = g_window.device_pixel_ratio;
            auto x = static_cast<float>(touch.targetX * dpr);
            auto y = static_cast<float>(touch.targetY * dpr);
            // zero for unknown
            auto id = static_cast<uint64_t>(touch.identifier + 1);
            switch (type) {
                case EMSCRIPTEN_EVENT_TOUCHSTART:
                    g_app.dispatch({touch_event_type::begin, id, x, y});
                    break;
                case EMSCRIPTEN_EVENT_TOUCHMOVE:
                    g_app.dispatch({touch_event_type::move, id, x, y});
                    break;
                case EMSCRIPTEN_EVENT_TOUCHEND:
                    g_app.dispatch({touch_event_type::end, id, x, y});
                    break;
                case EMSCRIPTEN_EVENT_TOUCHCANCEL:
                    g_app.dispatch({touch_event_type::end, id, x, y});
                    break;
                default:
                    break;
            }
        }
    }

    return true;
}

void subscribe_input() {
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
    const auto dpr = emscripten_get_device_pixel_ratio();

    double css_w, css_h;
    emscripten_get_element_css_size(DIV_ID, &css_w, &css_h);

    // TODO: configurable min aspect (70/100)
    // TODO: landscape and different modes, native letterbox
    const float aspect = g_window.creation_config.width / g_window.creation_config.height;
    int w = css_w;
    int h = css_h;
    int offset_x = 0;
    int offset_y = 0;
    if (g_window.creation_config.landscape) {
        h = std::min(h, static_cast<int>(w / aspect));
        offset_y = (css_h - h) / 2;
    } else {
        w = std::min(w, static_cast<int>(h * aspect));
        offset_x = (css_w - w) / 2;
    };

    css_w = w;
    css_h = h;

    g_window.device_pixel_ratio = dpr;

    g_window.window_size = {
            static_cast<uint32_t>(css_w),
            static_cast<uint32_t>(css_h)
    };

    g_window.back_buffer_size = {
            static_cast<uint32_t>(css_w * dpr),
            static_cast<uint32_t>(css_h * dpr)
    };

    emscripten_set_canvas_element_size(CANVAS_ID,
                                       static_cast<int>(g_window.back_buffer_size.width),
                                       static_cast<int>(g_window.back_buffer_size.height));

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
    subscribe_input();
    init_webgl_context();

    g_app.init();
    g_app.start();

    emscripten_set_main_loop(&loop, 0, 1);

    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1); // vsync
    //emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 1); // no-vsync?
}

void window_t::update_mouse_cursor() {
    const char* cursor_name = "default";
    switch (cursor_) {
        case mouse_cursor_t::button:
            cursor_name = "pointer";
            break;
        case mouse_cursor_t::help:
            cursor_name = "help";
            break;
        case mouse_cursor_t::parent:
            cursor_name = "auto";
            break;
        case mouse_cursor_t::arrow:
            cursor_name = "default";
            break;
        default:
            break;
    }

    emscripten_set_canvas_cursor(cursor_name);
}

void application_t::exit(int) {
    EM_ASM({window.close()}, 0);
}

}

// SHARING
namespace ek {

void sharing_navigate(const char* url) {
    EM_ASM({window.open(UTF8ToString($0), "_blank")}, url);
}

void sharing_rate_us(const char* appId) {
    (void) appId;
}

void sharing_send_message(const char* text) {
    (void) text;
}

}