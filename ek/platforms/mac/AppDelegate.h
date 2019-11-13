#import <Cocoa/Cocoa.h>
#import "OpenGLView.h"

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate> {
@public
    __strong NSWindow* nsWindow;
    __strong OpenGLView* openGLView;
}
@end

extern AppDelegate* gAppDelegate;

