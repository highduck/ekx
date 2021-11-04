#include <ek/app/Platform.h>
#include <ek/app/app.hpp>

#if TARGET_OS_IOS || TARGET_OS_TV

#include "impl/app_delegate_ios.h"

#else

#include "impl/app_delegate_macos.h"

#endif

#import "impl/device_apple.h"
#import "impl/sharing_apple.h"

AppDelegate* gAppDelegate = nullptr;

#ifndef EK_NO_MAIN
int main(int argc, char* argv[]) {
    using namespace ek::app;
    g_app.argc = argc;
    g_app.argv = argv;

#if TARGET_OS_IOS || TARGET_OS_TV
    @autoreleasepool {
        UIApplicationMain(argc, argv, nil, NSStringFromClass(AppDelegate.class));
    }
#else
    @autoreleasepool {
        [NSApplication.sharedApplication setDelegate:[AppDelegate new]];
        [NSApplication.sharedApplication run];
    }
#endif
    return 0;
}
#endif

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
