#include <TargetConditionals.h>
//#import <CoreFoundation/CoreFoundation.h>
#include <ek/app/app.hpp>

#if TARGET_OS_IOS || TARGET_OS_TV

#import <ios_app_delegate.h>

#else

#import <mac_app_delegate.h>

#endif

int main(int argc, char* argv[]) {
    ::ek::g_app.args = {argc, argv};
    ::ek::main();
    return 0;
}

namespace ek {

void start_application() {
#if TARGET_OS_IOS || TARGET_OS_TV
    int argc = g_app.args.argc;
    char** argv = g_app.args.argv;
    @autoreleasepool {
        UIApplicationMain(argc, argv, nil, NSStringFromClass(IOSAppDelegate.class));
    }
#else
    @autoreleasepool {
        [NSApplication.sharedApplication setDelegate:[MacAppDelegate new]];
        [NSApplication.sharedApplication run];
    }
#endif
}

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
