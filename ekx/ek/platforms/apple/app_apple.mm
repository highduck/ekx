#include "app_apple.h"
#include <ek/app/app.hpp>
#include <ek/Arguments.hpp>

using namespace ek::app;

#if TARGET_OS_IOS || TARGET_OS_TV

#import "impl/app_delegate_ios.h"

#else

#import "impl/app_delegate_macos.h"

#endif

#import "impl/device_apple.h"
#import "impl/resources_apple.h"
#import "impl/user_prefs_apple.h"
#import "impl/sharing_apple.h"
#import "impl/analytics_apple.h"

AppDelegate* gAppDelegate = nullptr;

#ifndef EK_NO_MAIN
int main(int argc, char* argv[]) {
    ::ek::Arguments::current = {argc, argv};
    ::ek::main();
    return 0;
}
#endif

namespace ek {

void start_application() {
#if TARGET_OS_IOS || TARGET_OS_TV
    int argc = args.argc;
    char** argv = args.argv;
    @autoreleasepool {
        UIApplicationMain(argc, argv, nil, NSStringFromClass(AppDelegate.class));
    }
#else
    @autoreleasepool {
        [NSApplication.sharedApplication setDelegate:[AppDelegate new]];
        [NSApplication.sharedApplication run];
    }
#endif
}

}

namespace ek::app {

void* getMetalDevice() {
    if(gAppDelegate.view != nil) {
        return (__bridge void*) [gAppDelegate view].device;
    }
    return nullptr;
}

const void* getMetalRenderPass() {
    if(gAppDelegate.view != nil) {
        return (__bridge const void*) (gAppDelegate.view.defaultPass);
    }
    return nullptr;
}

const void* getMetalDrawable() {
    if(gAppDelegate.view != nil) {
        return (__bridge const void*) (gAppDelegate.view.currentDrawable);
    }
    return nullptr;
}

}
