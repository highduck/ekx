#import "mac_app_delegate.h"

MacAppDelegate* g_app_delegate = nullptr;

namespace ek {

void start_application() {
#if defined(__objc_arc)
    [NSAutoreleasePool new];
#endif
    auto* app = NSApplication.sharedApplication;
    g_app_delegate = [[MacAppDelegate alloc] init];
    [app setDelegate:g_app_delegate];
    [app run];
}

}
