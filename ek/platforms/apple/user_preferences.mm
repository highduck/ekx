#include <ek/app/prefs.hpp>

#if defined(__APPLE__)

#import <Foundation/Foundation.h>

namespace ek {

void set_user_preference(const std::string& key, int value) {
    assert(!key.empty());

    NSString* ns_key = [NSString stringWithUTF8String:key.c_str()];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    [user_defaults setInteger:value forKey:ns_key];
    [user_defaults synchronize];
}

int get_user_preference(const std::string& key, int default_value) {
    assert(!key.empty());

    NSString* ns_key = [NSString stringWithUTF8String:key.c_str()];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    if ([user_defaults objectForKey:ns_key] != nil) {
        return static_cast<int>([user_defaults integerForKey:ns_key]);
    }

    return default_value;
}

void set_user_string(const std::string& key, const std::string& str) {
    assert(!key.empty());

    NSString* ns_key = [NSString stringWithUTF8String:key.c_str()];
    NSString* ns_value = [NSString stringWithUTF8String:str.c_str()];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    [user_defaults setObject:ns_value forKey:ns_key];
    [user_defaults synchronize];
}

std::string get_user_string(const std::string& key, const std::string& default_value) {
    assert(!key.empty());

    NSString* ns_key = [NSString stringWithUTF8String:key.c_str()];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    if ([user_defaults objectForKey:ns_key] != nil) {
        NSString* ns_result = [user_defaults stringForKey:ns_key];
        return std::string{[ns_result UTF8String]};
    }

    return default_value;
}

}

#endif