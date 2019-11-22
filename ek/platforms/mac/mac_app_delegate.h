#import <Cocoa/Cocoa.h>
#import "mac_opengl_view.h"

@interface MacAppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate> {
@public
    __strong NSWindow* window;
    __strong MacOpenGLView* gl_view;
}
@end

extern MacAppDelegate* g_app_delegate;

