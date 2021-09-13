#pragma once

#include <ek/app/app.hpp>

extern "C" {
extern void web_vibrate(int cur);

/**
 *
 * @param dest - buffer to write the language code string
 * @param max - buffer size
 * @return destination chars pointer, or nullptr in case of error
 */
extern const char* web_get_lang(char* dest, int max);
}

namespace ek::app {

const float* getScreenInsets() {
    return nullptr;
}

void vibrate(int duration_millis) {
    web_vibrate(duration_millis);
}

const char* getPreferredLang() {
    static char lang[8];
    return web_get_lang(lang, 8);
}

const char* getSystemFontPath(const char* fontName) {
    (void)fontName;
    // TODO:
    return nullptr;
}

}