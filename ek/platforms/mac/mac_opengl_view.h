#import <Cocoa/Cocoa.h>

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

@interface MacOpenGLView : NSOpenGLView {
    CVDisplayLinkRef displayLink;
}

@end
