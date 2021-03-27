#include <ek/app/app.hpp>
#include <TargetConditionals.h>

#if TARGET_OS_IOS || TARGET_OS_TV

#import <ios_app_delegate.h>

#else

#import <mac_app_delegate.h>
#include "apple_common.h"

#endif

using namespace ek::app;

int main(int argc, char* argv[]) {
    g_app.args = {argc, argv};
    ::ek::main();
    return 0;
}

namespace ek {

void start_application() {
    // TODO: ios/macos support creation flags
    //  `g_app.window_cfg.needDepth`

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

namespace apple {

void handle_exit_request() {
    if (g_app.require_exit) {
        g_app.require_exit = false;
        ::exit(g_app.exit_code);
    }
}

}
}
