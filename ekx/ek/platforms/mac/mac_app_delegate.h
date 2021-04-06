#import <MetalKit/MetalKit.h>

@interface MacOSView : MTKView {}
@end

@interface MacAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>

@property(strong, nonatomic) NSApplication* application;
@property(strong, nonatomic) NSWindow* window;
@property(strong, nonatomic) MacOSView* view;

@end

extern MacAppDelegate* gAppDelegate;