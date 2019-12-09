#import <Cocoa/Cocoa.h>
#import "mac_opengl_view.h"

@interface MacAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>

@property(strong, nonatomic) NSApplication* application;
@property(strong, nonatomic) NSWindow* window;
@property(strong, nonatomic) MacOpenGLView* view;

@end

extern MacAppDelegate* g_app_delegate;

