#include <ek/app/app.hpp>
#include <ek/util/logger.hpp>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <unordered_map>

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
    attrs.depth = true;
    attrs.stencil = false;
    attrs.antialias = false;

    webgl_context = emscripten_webgl_create_context(CANVAS_ID, &attrs);

    if (!webgl_context) {
        EK_ERROR << "Failed to create WebGL context";
        return;
    }

    EMSCRIPTEN_RESULT result = emscripten_webgl_make_context_current(webgl_context);
    if (result != EMSCRIPTEN_RESULT_SUCCESS) {
        EK_ERROR << "Failed to make WebGL context current";
        return;
    }
}

mouse_button convert_mouse_button(unsigned short btn) {
    switch (btn) {
        case 0:
            return mouse_button::left;
        case 2:
            return mouse_button::right;
        default:
            break;
    }
    return mouse_button::other;
}

static EM_BOOL em_mouse_callback(int type, const EmscriptenMouseEvent* mouse_event, void*) {
    event_t event{};
    event.button = convert_mouse_button(mouse_event->button);

    switch (type) {
        case EMSCRIPTEN_EVENT_MOUSEDOWN:
            event.type = event_type::mouse_down;
            break;
        case EMSCRIPTEN_EVENT_MOUSEUP:
            event.type = event_type::mouse_up;
            break;
        case EMSCRIPTEN_EVENT_MOUSEMOVE:
            event.type = event_type::mouse_move;
            break;
        default:
            return false;
    }

    event.set_position(static_cast<float>(mouse_event->targetX),
                       static_cast<float>(mouse_event->targetY),
                       g_window.device_pixel_ratio);
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

static key_code convert_key_code(const EM_UTF8 key_code[EM_HTML5_SHORT_STRING_LEN_BYTES]) {
    const auto i = scancode_table.find(key_code);
    return i != scancode_table.end() ? i->second : key_code::Unknown;
}

static EM_BOOL em_keyboard_callback(int type, const EmscriptenKeyboardEvent* event, void*) {
    event_t ev{};
    switch (type) {
        case EMSCRIPTEN_EVENT_KEYPRESS:
            ev.type = event_type::key_press;
            ev.code = convert_key_code(event->code);
            g_app.dispatch(ev);
            return true;
        case EMSCRIPTEN_EVENT_KEYDOWN:
            ev.type = event_type::key_down;
            ev.code = convert_key_code(event->code);
            g_app.dispatch(ev);
            return true;
        case EMSCRIPTEN_EVENT_KEYUP:
            ev.type = event_type::key_up;
            ev.code = convert_key_code(event->code);
            g_app.dispatch(ev);
            return true;
        default:
            break;
    }

    return false;
}

static EM_BOOL em_wheel_callback(int type, const EmscriptenWheelEvent* event, void*) {
    if (type == EMSCRIPTEN_EVENT_WHEEL) {
        event_t ev{event_type::mouse_scroll};
        ev.set_position(static_cast<float>(event->mouse.targetX),
                        static_cast<float>(event->mouse.targetY),
                        g_window.device_pixel_ratio);
        ev.set_mouse_scroll(event->deltaX, event->deltaY);
        g_app.dispatch(ev);
    }
    return true;
}


static EM_BOOL em_touch_callback(int type, const EmscriptenTouchEvent* event, void*) {
    event_t ev;
    const float dpr = g_window.device_pixel_ratio;
    for (int i = 0; i < event->numTouches; ++i) {
        const EmscriptenTouchPoint& touch = event->touches[i];
        if (touch.isChanged) {
            ev.id = static_cast<uint64_t>(touch.identifier) + 1;
            ev.set_position(static_cast<float>(touch.targetX),
                            static_cast<float>(touch.targetY),
                            dpr);
            switch (type) {
                case EMSCRIPTEN_EVENT_TOUCHSTART:
                    ev.type = event_type::touch_begin;
                    g_app.dispatch(ev);
                    break;
                case EMSCRIPTEN_EVENT_TOUCHMOVE:
                    ev.type = event_type::touch_move;
                    g_app.dispatch(ev);
                    break;
                case EMSCRIPTEN_EVENT_TOUCHEND:
                    ev.type = event_type::touch_end;
                    g_app.dispatch(ev);
                    break;
                case EMSCRIPTEN_EVENT_TOUCHCANCEL:
                    ev.type = event_type::touch_end;
                    g_app.dispatch(ev);
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
//    emscripten_set_pointerlockchange_callback(nullptr, nullptr, true, em_pointer_lock_change_callback);

    emscripten_set_touchstart_callback(CANVAS_ID, nullptr, false, em_touch_callback);
    emscripten_set_touchend_callback(CANVAS_ID, nullptr, false, em_touch_callback);
    emscripten_set_touchmove_callback(CANVAS_ID, nullptr, false, em_touch_callback);
    emscripten_set_touchcancel_callback(CANVAS_ID, nullptr, false, em_touch_callback);

    emscripten_set_keypress_callback(nullptr, nullptr, true, em_keyboard_callback);
    emscripten_set_keydown_callback(nullptr, nullptr, true, em_keyboard_callback);
    emscripten_set_keyup_callback(nullptr, nullptr, true, em_keyboard_callback);
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
                                       static_cast<int>(g_window.drawable_size.x),
                                       static_cast<int>(g_window.drawable_size.y));

    emscripten_set_element_css_size(CANVAS_ID, css_w, css_h);

    EM_ASM({
               document.getElementById("gameview").style["transform"] =
                       "translateX(" + $0 + "px) translateY(" + $1 + "px)";
           }, offset_x, offset_y);
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

EM_JS(void, emscripten_set_canvas_cursor, (const char* type),

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
    EM_ASM({ window.close() }, 0);
}

}

// SHARING
namespace ek {

void sharing_navigate(const char* url) {
    EM_ASM({ window.open(UTF8ToString($0), "_blank") }, url);
}

void sharing_rate_us(const char* appId) {
    (void) appId;
}

void sharing_send_message(const char* text) {
    (void) text;
}

}