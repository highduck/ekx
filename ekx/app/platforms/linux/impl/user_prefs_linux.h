#pragma once

#include <ek/app/prefs.hpp>
#include <ek/assert.hpp>

#define C_STRING_NOT_NULL_OR_EMPTY(x) ((x) != nullptr && *(x) != '\0')
#define ASSERT_KEY_IS_VALID(x) EK_ASSERT(C_STRING_NOT_NULL_OR_EMPTY(x))

namespace ek {

void set_user_preference(const char* key, int value) {
    ASSERT_KEY_IS_VALID(key);
}

int get_user_preference(const char* key, int default_value) {
    ASSERT_KEY_IS_VALID(key);
    return default_value;
}

void set_user_string(const char* key, const char* str) {
    ASSERT_KEY_IS_VALID(key);
}

std::string get_user_string(const char* key, const char* default_value) {
    ASSERT_KEY_IS_VALID(key);
    return default_value;
}

}