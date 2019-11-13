#import "AppDelegate.h"

AppDelegate* gAppDelegate = nullptr;

namespace ek {

void start_application() {
#if defined(__objc_arc)
    [NSAutoreleasePool new];
#endif
    auto* app = NSApplication.sharedApplication;
    gAppDelegate = [[AppDelegate alloc] init];
    [app setDelegate: gAppDelegate];
    [app run];
}

}
