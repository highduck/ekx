#ifndef EK_APP_H
#define EK_APP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ek_app_event_type {
    EK_APP_EVENT_RESUME = 0,
    EK_APP_EVENT_PAUSE,
    EK_APP_EVENT_RESIZE,
    EK_APP_EVENT_BACK_BUTTON,
    EK_APP_EVENT_CLOSE,

    EK_APP_EVENT_TOUCH_START = 5,
    EK_APP_EVENT_TOUCH_MOVE,
    EK_APP_EVENT_TOUCH_END,

    EK_APP_EVENT_MOUSE_MOVE = 8,
    EK_APP_EVENT_MOUSE_DOWN,
    EK_APP_EVENT_MOUSE_UP,
    EK_APP_EVENT_MOUSE_ENTER,
    EK_APP_EVENT_MOUSE_EXIT,

    EK_APP_EVENT_WHEEL = 13,

    EK_APP_EVENT_KEY_DOWN = 14,
    EK_APP_EVENT_KEY_UP,
    // TODO: since keypress deprecated on web, check if we need it
    // TODO: `KeyPress` macro pollution from X11 headers
    EK_APP_EVENT_KEY_PRESS,

    EK_APP_EVENT_TEXT = 17,

    EK_APP_EVENT_COUNT,
} ek_app_event_type;

typedef enum ek_mouse_cursor {
    EK_MOUSE_CURSOR_PARENT = 0,
    EK_MOUSE_CURSOR_ARROW,
    EK_MOUSE_CURSOR_BUTTON,
    EK_MOUSE_CURSOR_HELP,
    EK_MOUSE_CURSOR_MAX
} ek_mouse_cursor;

typedef enum ek_mouse_button {
    EK_MOUSE_BUTTON_LEFT,
    EK_MOUSE_BUTTON_RIGHT,
    EK_MOUSE_BUTTON_OTHER,
} ek_mouse_button;

typedef enum ek_key_code {
    EK_KEYCODE_UNKNOWN = 0,

    EK_KEYCODE_ARROW_UP,
    EK_KEYCODE_ARROW_DOWN,
    EK_KEYCODE_ARROW_LEFT,
    EK_KEYCODE_ARROW_RIGHT,
    EK_KEYCODE_ESCAPE,
    EK_KEYCODE_SPACE,
    EK_KEYCODE_ENTER,
    EK_KEYCODE_BACKSPACE,

    // extra
    EK_KEYCODE_TAB,
    EK_KEYCODE_PAGEUP,
    EK_KEYCODE_PAGEDOWN,
    EK_KEYCODE_HOME,
    EK_KEYCODE_END,
    EK_KEYCODE_INSERT,
    EK_KEYCODE_DELETE,

    EK_KEYCODE_A,
    EK_KEYCODE_C,
    EK_KEYCODE_V,
    EK_KEYCODE_X,
    EK_KEYCODE_Y,
    EK_KEYCODE_Z,
    EK_KEYCODE_W,
    EK_KEYCODE_S,
    EK_KEYCODE_D,
    EK_KEYCODE_MAX_COUNT
} ek_key_code;

typedef enum ek_key_mod {
    EK_KEY_MOD_NONE = 0,
    // Super is "command" or "windows" key
    EK_KEY_MOD_SUPER = 1,
    EK_KEY_MOD_SHIFT = 2,
    EK_KEY_MOD_CONTROL = 4,
    EK_KEY_MOD_ALT = 8
} ek_key_mod;

typedef struct ek_app_text_event {
    ek_app_event_type type;
    char data[8];
} ek_app_text_event;

typedef struct ek_app_key_event {
    ek_app_event_type type;
    ek_key_code code;
    ek_key_mod modifiers;
} ek_app_key_event;

typedef struct ek_app_touch_event {
    ek_app_event_type type;
    uint64_t id;
    float x;
    float y;
} ek_app_touch_event;

typedef struct ek_app_mouse_event {
    ek_app_event_type type;
    ek_mouse_button button;
    float x;
    float y;
} ek_app_mouse_event;

typedef struct ek_app_wheel_event {
    ek_app_event_type type;
    float x;
    float y;
} ek_app_wheel_event;

typedef union ek_app_event {
    ek_app_event_type type;
    ek_app_key_event key;
    ek_app_text_event text;
    ek_app_touch_event touch;
    ek_app_mouse_event mouse;
    ek_app_wheel_event wheel;
} ek_app_event;

typedef struct ek_app_config {
    const char* title; //
    float width;// = 960;
    float height;// = 720;
    bool need_depth;// = false;
    bool web_keep_canvas_aspect_ratio;// = false;
    bool allow_high_dpi;// = true;
    int sample_count;// = 1;
    int swap_interval;// = 1;
    uint32_t background_color; //0x0

    // for windowed systems, currently only macOS
    // in logical-pixel units
    // by default 0 - auto size from designed width / height
    int window_x;
    int window_y;
    int window_width;
    int window_height;
} ek_app_config;

typedef struct ek_app_viewport {
    /**
     * backing surface size
     * in pixels
     */
    float width;
    float height;
    /**
     * DPI scale (divisor between logical and backing pixel unit)
     * `backing_pixel = scale * logical_pixel`
     * for example logical window width will be (width / scale)
     */
    // TODO: scaleX / scaleY ?
    float scale;
    /**
     * Safe area margins left-top-right-bottom
     * in pixels
     */
    float insets[4];
} ek_app_viewport;

typedef enum ek_app_state_flag {
    EK_APP_STATE_READY = 1 << 0,
    EK_APP_STATE_RUNNING = 1 << 1,
    EK_APP_STATE_FULLSCREEN = 1 << 2,
    EK_APP_STATE_EXIT_PENDING = 1 << 3,
    EK_APP_STATE_SIZE_CHANGED = 1 << 4,
    EK_APP_STATE_CURSOR_CHANGED = 1 << 5,
} ek_app_state_flag;

typedef struct ek_app_context {
    void (* on_ready)(void);

    void (* on_frame)(void);

    void (* on_event)(const ek_app_event);

    char** argv;

    int argc;

    ek_app_config config;

    ek_app_viewport viewport;

    int exit_code; // 0

    ek_mouse_cursor cursor; /* EK_MOUSE_CURSOR_PARENT */

    ek_app_state_flag state;

    /**
     * Default lang on device during application creation (currently default for user on the device)
     */
    char lang[8];

} ek_app_context;

extern ek_app_context ek_app;

extern void ek_app_main(void);

void ek_app_quit(int status);

void ek_app_cancel_quit(void);

void ek_app_set_mouse_cursor(ek_mouse_cursor cursor);

int ek_app_open_url(const char* url);

int ek_app_font_path(char* dest, uint32_t size, const char* font_name);

int ek_app_share(const char* content);

#ifdef __APPLE__

void* ek_app_mtl_device(void);

const void* ek_app_mtl_render_pass(void);

const void* ek_app_mtl_drawable(void);

const char* ek_app_ns_bundle_path(const char* path, char* buffer, uint32_t size);

#endif

#ifdef __cplusplus
}
#endif

#endif // EK_APP_H
