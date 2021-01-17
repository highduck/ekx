#include <string>
#include <array>
#include <ek/math/vec.hpp>

#include <TargetConditionals.h>

#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#import <AudioToolbox/AudioToolbox.h>

#if TARGET_OS_IOS || TARGET_OS_TV


#else


#endif

namespace ek {

std::string get_device_lang() {
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

float4 get_screen_insets() {
    float4 result{};
#if TARGET_OS_IOS || TARGET_OS_TV

#endif
    return result;
}

void vibrate(int duration_millis) {
    (void)(duration_millis);
    AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

std::string getDeviceFontPath(const char* fontName) {
    CFStringRef targetName = CFStringCreateWithCString(nullptr, fontName, kCFStringEncodingUTF8);
    CTFontDescriptorRef targetDescriptor = CTFontDescriptorCreateWithNameAndSize(targetName, 0.0);
    CFURLRef targetURL = (CFURLRef) CTFontDescriptorCopyAttribute(targetDescriptor, kCTFontURLAttribute);
    std::string fontPath;

    if (targetURL) {
        UInt8 buffer[PATH_MAX];
        CFURLGetFileSystemRepresentation(targetURL, true, buffer, PATH_MAX);
        fontPath = std::string((char*) buffer);
        CFRelease(targetURL);
    }

    CFRelease(targetName);
    CFRelease(targetDescriptor);

    return fontPath;
}

}
