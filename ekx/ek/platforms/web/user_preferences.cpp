#include <ek/app/prefs.hpp>
#include <ek/assert.hpp>
#include <emscripten.h>

extern "C" {

extern void web_prefs_set_number(const char* key, double value);
extern double web_prefs_get_number(const char* key, double default_value);
extern void web_prefs_set_string(const char* key, const char* value);
extern char* web_prefs_get_string(const char* key);

}

namespace ek {

void set_user_preference(const char* key, int value) {
    web_prefs_set_number(key, value);
}

int get_user_preference(const char* key, int default_value) {
    return web_prefs_get_number(key, default_value);
}

void set_user_string(const char* key, const char* value) {
    web_prefs_set_string(key, value);
}

std::string get_user_string(const char* key, const char* default_value) {
    char* buffer = web_prefs_get_string(key);
    if (buffer) {
        std::string str{buffer};
        free(buffer);
        return str;
    }
    return std::string{default_value};
}

}