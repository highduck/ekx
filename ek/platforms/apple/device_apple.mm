#include <string>
#include <array>
#include <TargetConditionals.h>

#import <CoreFoundation/CoreFoundation.h>
#import <AudioToolbox/AudioToolbox.h>

#if TARGET_OS_IOS || TARGET_OS_TV


#else


#endif

namespace ek {
[[maybe_unused]] std::string get_device_lang() {
    std::string result;
    CFLocaleRef locale = CFLocaleCopyCurrent();
    auto value = static_cast<CFStringRef>(CFLocaleGetValue(locale, kCFLocaleLanguageCode));
    const size_t buffer_size = 32;
    char buffer[buffer_size];
    CFStringEncoding encoding = kCFStringEncodingUTF8;
    if (CFStringGetCString(value, buffer, buffer_size, encoding)) {
        result = buffer;
    }
    CFRelease(locale);
    return result;
}

std::array<int, 4> get_screen_insets() {
    std::array<int, 4> result{0, 0, 0, 0};
#if TARGET_OS_IOS || TARGET_OS_TV

#endif
    return result;
}

void vibrate(int duration_millis) {
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

}