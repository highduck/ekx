#pragma once

#include <ek/assert.hpp>
#import <Foundation/Foundation.h>

#define C_STRING_NOT_NULL_OR_EMPTY(x) ((x) != nullptr && *(x) != '\0')
#define ASSERT_KEY_IS_VALID(x) EK_ASSERT(C_STRING_NOT_NULL_OR_EMPTY(x))

namespace ek {

void set_user_preference(const char* key, int value) {
    ASSERT_KEY_IS_VALID(key);

    NSString* ns_key = [NSString stringWithUTF8String:key];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    [user_defaults setInteger:value forKey:ns_key];
    [user_defaults synchronize];
}

int get_user_preference(const char* key, int default_value) {
    ASSERT_KEY_IS_VALID(key);

    NSString* ns_key = [NSString stringWithUTF8String:key];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    if ([user_defaults objectForKey:ns_key] != nil) {
        return static_cast<int>([user_defaults integerForKey:ns_key]);
    }

    return default_value;
}

void set_user_string(const char* key, const char* str) {
    ASSERT_KEY_IS_VALID(key);

    NSString* ns_key = [NSString stringWithUTF8String:key];
    NSString* ns_value = [NSString stringWithUTF8String:str];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    [user_defaults setObject:ns_value forKey:ns_key];
    [user_defaults synchronize];
}

std::string get_user_string(const char* key, const char* default_value) {
    ASSERT_KEY_IS_VALID(key);

    NSString* ns_key = [NSString stringWithUTF8String:key];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    if ([user_defaults objectForKey:ns_key] != nil) {
        NSString* ns_result = [user_defaults stringForKey:ns_key];
        return std::string{[ns_result UTF8String]};
    }

    return std::string{default_value};
}

}
