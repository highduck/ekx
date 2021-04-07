#import <MetalKit/MetalKit.h>
#include <TargetConditionals.h>

#if __has_feature(objc_arc)
#define OBJC_RELEASE(obj) { obj = nil; }
#else
#define OBJC_RELEASE(obj) { [obj release]; obj = nil; }
#endif

// we use common metal view for iOS and macOS
@interface MetalView : MTKView {}
@end

#if TARGET_OS_IOS || TARGET_OS_TV

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property(strong, nonatomic) UIWindow* window;
@property(strong, nonatomic) MetalView* view;

@end

#else

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>

@property(strong, nonatomic) NSApplication* application;
@property(strong, nonatomic) NSWindow* window;
@property(strong, nonatomic) MetalView* view;

@end

#endif

extern AppDelegate* gAppDelegate;
