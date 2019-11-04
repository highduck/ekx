#import <Cocoa/Cocoa.h>

@interface OpenGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink;
}

@end
