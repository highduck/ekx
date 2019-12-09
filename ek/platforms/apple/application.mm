#include <ek/app/app.hpp>

#import <CoreFoundation/CoreFoundation.h>

namespace ek {

void application_t::exit(int code) {
    ::exit(code);
}

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

}
