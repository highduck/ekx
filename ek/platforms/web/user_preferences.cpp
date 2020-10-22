#include <ek/util/base64.hpp>
#include <cassert>
#include <emscripten.h>

extern "C" {

extern void web_prefs_set_number(const char* key, double value);
extern double web_prefs_get_number(const char* key, double default_value);
extern void web_prefs_set_string(const char* key, const char* value);
extern char* web_prefs_get_string(const char* key);

}

namespace ek {

void set_user_preference(const std::string& key, int value) {
    web_prefs_set_number(key.c_str(), value);
}

int get_user_preference(const std::string& key, int default_value) {
    return web_prefs_get_number(key.c_str(), default_value);
}

void set_user_string(const std::string& key, const std::string& value) {
    web_prefs_set_string(key.c_str(), value.c_str());
}

std::string get_user_string(const std::string& key, const std::string& default_value) {
    char* buffer = web_prefs_get_string(key.c_str());
    if (buffer) {
        std::string str{buffer};
        free(buffer);
        return str;
    }
    return default_value;
}

void set_user_data(const std::string& key, const std::vector<uint8_t>& buffer) {
    assert(!key.empty());
    set_user_string(key, ek::base64::encode(buffer));
}

std::vector<uint8_t> get_user_data(const std::string& key) {
    return ek::base64::decode(get_user_string(key, ""));
}

}