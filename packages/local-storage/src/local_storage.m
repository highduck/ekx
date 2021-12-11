#ifdef __APPLE__

#include <ek/app.h>
#include <ek/assert.h>

#import <Foundation/Foundation.h>

#define C_STRING_NOT_NULL_OR_EMPTY(x) ((x) != 0 && *(x) != 0)
#define ASSERT_KEY_IS_VALID(x) EK_ASSERT(C_STRING_NOT_NULL_OR_EMPTY(x))

void ek_ls_set_i(const char* key, int value) {
    ASSERT_KEY_IS_VALID(key);

    NSString* ns_key = [NSString stringWithUTF8String:key];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    [user_defaults setInteger:value forKey:ns_key];
    [user_defaults synchronize];
}

int ek_ls_get_i(const char* key, int de_fault) {
    ASSERT_KEY_IS_VALID(key);

    NSString* ns_key = [NSString stringWithUTF8String:key];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    if ([user_defaults objectForKey:ns_key] != nil) {
        return (int)([user_defaults integerForKey:ns_key]);
    }

    return de_fault;
}

void ek_ls_set_s(const char* key, const char* str) {
    ASSERT_KEY_IS_VALID(key);

    NSString* ns_key = [NSString stringWithUTF8String:key];
    NSString* ns_value = [NSString stringWithUTF8String:str];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    [user_defaults setObject:ns_value forKey:ns_key];
    [user_defaults synchronize];
}

int ek_ls_get_s(const char* key, char* buffer, uint32_t buffer_size) {
    ASSERT_KEY_IS_VALID(key);
    EK_ASSERT(buffer_size > 0);
    EK_ASSERT(buffer != nil);

    buffer[0] = 0;

    NSString* ns_key = [NSString stringWithUTF8String:key];
    NSUserDefaults* user_defaults = [NSUserDefaults standardUserDefaults];
    if ([user_defaults objectForKey:ns_key] != nil) {
        NSString* ns_result = [user_defaults stringForKey:ns_key];
        const char* res = [ns_result UTF8String];
        strncpy(buffer, res, buffer_size - 1);
        return (int)strnlen(buffer, buffer_size);
    }
    return 0;
}

#endif // __APPLE__
