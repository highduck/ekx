#include <ek/app.h>
#include <ek/log.h>
#include <ek/time.h>
#include <ek/rnd.h>

static uint32_t rnd;

static void log_viewport_props(void) {
    log_info("screen: %d x %d ( %d%% )",
             (int) ek_app.viewport.width,
             (int) ek_app.viewport.height,
             (int) (ek_app.viewport.scale * 100.0f));

    log_info("safe insets: [ %d, %d, %d, %d ]",
             (int) ek_app.viewport.insets[0],
             (int) ek_app.viewport.insets[1],
             (int) ek_app.viewport.insets[2],
             (int) ek_app.viewport.insets[3]);
}

static void on_ready(void) {
    log_info("lang: %s", ek_app.lang);

    char buf[1024];
    ek_app_font_path(buf, 1024, "_system");
    log_info("system font path: %s", buf);

    log_viewport_props();
}

static void on_frame(void) {
    log_tick();
}

static void on_event(const ek_app_event ev) {
    switch (ev.type) {
        case EK_APP_EVENT_TEXT:
            log_info("text");
            break;
        case EK_APP_EVENT_KEY_UP:
            log_info("key up");
            break;
        case EK_APP_EVENT_KEY_DOWN:
            log_info("key down %d", ev.key.code);
            if(ev.key.code == EK_KEYCODE_SPACE) {
                ek_app_set_mouse_cursor(ek_rand1(&rnd) % _EK_MOUSE_CURSOR_NUM);
            }
            else if(ev.key.code == EK_KEYCODE_S) {
                const char* text = "Hello, World! 👋";
                if(0 != ek_app_share(text)) {
                    log_warn("sharing api is not supported: %s", text);
                }
            }
            else if(ev.key.code == EK_KEYCODE_D) {
                ek_app_open_url("https://github.com/eliasku/ek");
            }
            else if(ev.key.code == EK_KEYCODE_ESCAPE) {
                static int exit_counter = 0;
                ek_app_quit(1);

                if(++exit_counter < 3) {
                    ek_app_cancel_quit();
                }
            }
            break;
        case EK_APP_EVENT_KEY_PRESS:
            log_info("key press");
            break;
        case EK_APP_EVENT_MOUSE_EXIT:
            log_info("mouse exit");
            break;
        case EK_APP_EVENT_MOUSE_ENTER:
            log_info("mouse enter");
            break;
        case EK_APP_EVENT_MOUSE_MOVE:
            log_info("mouse move");
            break;
        case EK_APP_EVENT_MOUSE_DOWN:
            log_info("mouse down");
            break;
        case EK_APP_EVENT_MOUSE_UP:
            log_info("mouse up");
            break;
        case EK_APP_EVENT_TOUCH_START:
            log_info("touch start");
            break;
        case EK_APP_EVENT_TOUCH_MOVE:
            log_info("touch move");
            break;
        case EK_APP_EVENT_TOUCH_END:
            log_info("touch end");
            break;
        case EK_APP_EVENT_PAUSE:
            log_info("pause");
            break;
        case EK_APP_EVENT_RESUME:
            log_info("resume");
            break;
        case EK_APP_EVENT_WHEEL:
            log_info("wheel");
            break;
        case EK_APP_EVENT_BACK_BUTTON:
            log_info("back button");
            break;
        case EK_APP_EVENT_CLOSE:
            log_info("close");
            break;
        case EK_APP_EVENT_RESIZE:
            log_info("resize");
            log_viewport_props();
            break;
        default:
            break;
    }
}

void ek_app_main() {
    ek_time_init();
    log_init();
    rnd = ek_time_seed32();

    ek_app.config.title = "Test";
    ek_app.config.width = 640;
    ek_app.config.height = 480;
    ek_app.on_ready = on_ready;
    ek_app.on_frame = on_frame;
    ek_app.on_event = on_event;
}