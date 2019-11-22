#import <Cocoa/Cocoa.h>

@interface MacOpenGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink;
}

@end
