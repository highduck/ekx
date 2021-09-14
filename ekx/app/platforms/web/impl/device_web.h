#pragma once

#include <ek/app/app.hpp>

extern "C" {

extern int ekapp_openURL(const char* url);
extern int ekapp_vibrate(int durationMillis);

/**
 *
 * @param dest - buffer to write the language code string
 * @param max - buffer size
 * @return destination chars pointer, or nullptr in case of error
 */
extern const char* ekapp_getLang(char* dest, int max);

}

namespace ek::app {

int openURL(const char* url) {
    return ekapp_openURL(url);
}

int vibrate(int duration_millis) {
    return ekapp_vibrate(duration_millis);
}

const float* getScreenInsets() {
    return nullptr;
}

const char* getPreferredLang() {
    static char lang[8];
    return ekapp_getLang(lang, 8);
}

const char* getSystemFontPath(const char* fontName) {
    (void) fontName;
    // TODO:
    return nullptr;
}

}