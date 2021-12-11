#ifndef EK_WEB_H
#define EK_WEB_H

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void ek_app_js_close(void);

extern void ek_app_js_set_mouse_cursor(int cur);

extern bool ek_app_js_init(int flags);

extern void ek_app_js_run(void);

/**
 *
 * @param dest - buffer to write the language code string
 * @param max - buffer size
 */
extern void ek_app_js_lang(char* dest, int max);

/**
 * open URL
 * @param url - c-string
 * @return 0 if success
 */
extern int ek_app_js_navigate(const char* url);

extern int ek_app_js_share(const char* content);

// callbacks from JS to C

bool ek_app_js__on_key(int type, int code, int modifiers);

bool ek_app_js__on_mouse(int type, int button, float x, float y);

bool ek_app_js__on_wheel(float x, float y);

bool ek_app_js__on_touch(int type, int id, float x, float y);

bool ek_app_js__on_resize(float w, float h, float dpr);

void ek_app_js__on_focus(int flags);

void ek_app_js__loop(void);



#ifdef __cplusplus
}
#endif

#endif // __EMSCRIPTEN__

#endif //EK_WEB_H
