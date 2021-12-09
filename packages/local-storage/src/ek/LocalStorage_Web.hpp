#pragma once

#include "LocalStorage.hpp"

extern "C" {

extern bool web_ls_set_f64(const char* key, double value);
extern bool web_ls_get_f64(const char* key, double* dest);
extern bool web_ls_set(const char* key, const char* value);
extern bool web_ls_get(const char* key, char* dest, int maxSize);

}

namespace ek {

void set_user_preference(const char* key, int value) {
    web_ls_set_f64(key, (double)value);
}

int get_user_preference(const char* key, int defaultValue) {
    double v = 0.0;
    if(web_ls_get_f64(key, &v)) {
        // possibly need correction
        return (int)v;
    }
    return defaultValue;
}

void set_user_string(const char* key, const char* value) {
    web_ls_set(key, value);
}

String get_user_string(const char* key, const char* defaultValue) {
    char buffer[4096];
    return web_ls_get(key, buffer, 4096) ? buffer : defaultValue;
}

}