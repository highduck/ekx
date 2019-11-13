#import "EAGLView.h"

#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#include <platform/application.hpp>

using namespace ek;

@interface EAGLView () {
    EAGLContext* _context;
    NSInteger _framesPerSecond;
    CADisplayLink* _displayLink;

    // The OpenGL names for the framebuffer and renderbuffer used to render to this view
    GLuint _colorRenderbuffer;
    GLuint _depthRenderbuffer;
    GLuint _defaultFBOName;
}
@end

@implementation EAGLView

// Create an ES 2.0 context
- (instancetype)createRenderer:(id <EAGLDrawable>)drawable {
    // Create default framebuffer object. The backing will be allocated for the
    // current layer in -resizeFromLayer

    glGenFramebuffers(1, &_defaultFBOName);

    glGenRenderbuffers(1, &_colorRenderbuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBOName);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);

    // This call associates the storage for the current render buffer with the
    // EAGLDrawable (our CAEAGLLayer) allowing us to draw into a buffer that
    // will later be rendered to the screen wherever the layer is (which
    // corresponds with our view).
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:drawable];

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);

    // Get the drawable buffer's width and height so we can create a depth buffer for the FBO
    GLint backingWidth;
    GLint backingHeight;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);

    // Create a depth buffer to use with our drawable FBO
    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return nil;
    }
    
    g_window.primary_frame_buffer = _defaultFBOName;

    return self;
}

- (BOOL)resizeFromLayer:(CAEAGLLayer*)layer {
    // The pixel dimensions of the CAEAGLLayer
    GLint backingWidth;
    GLint backingHeight;

    // Allocate color buffer backing based on the current layer size
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);

    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);

    g_window.back_buffer_size = {
        static_cast<uint32_t>(backingWidth),
        static_cast<uint32_t>(backingHeight)
    };
    
    g_window.size_changed = true;

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return NO;
    }

    return YES;
}


// Must return the CAEAGLLayer class so that CA allocates an EAGLLayer backing for this view
+ (Class)layerClass {
    return [CAEAGLLayer class];
}

// The GL view is stored in the storyboard file. When it's unarchived it's sent -initWithCoder:
- (instancetype)initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        self.userInteractionEnabled = YES;
        self.multipleTouchEnabled = YES;
    
        // Get the layer
        CAEAGLLayer* eaglLayer = (CAEAGLLayer*) self.layer;

        eaglLayer.contentsScale = UIScreen.mainScreen.nativeScale;
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                [NSNumber numberWithBool:FALSE],
                kEAGLDrawablePropertyRetainedBacking,
                kEAGLColorFormatRGBA8,
                kEAGLDrawablePropertyColorFormat,
                        nil];


        _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

        if (!_context || ![EAGLContext setCurrentContext:_context]) {
            return nil;
        }

        [self createRenderer:(id <EAGLDrawable>) self.layer];

        _animating = FALSE;
        _framesPerSecond = 60;
        _displayLink = nil;
    }

    return self;
}

- (void)drawView:(id)sender {
    [EAGLContext setCurrentContext:_context];
    glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBOName);

    g_app.dispatch_draw_frame();

    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [_context presentRenderbuffer:GL_RENDERBUFFER];

}

- (void)layoutSubviews {
    [self resizeFromLayer:(CAEAGLLayer*) self.layer];
    [self drawView:nil];
}

- (NSInteger)framesPerSecond {
    return _framesPerSecond;
}

- (void)setFramesPerSecond:(NSInteger)framesPerSecond {
    // Frame interval defines how many display frames must pass between each time the
    // display link fires. The display link will only fire 30 times a second when the
    // frame internal is two on a display that refreshes 60 times a second. The default
    // frame interval setting of one will fire 60 times a second when the display refreshes
    // at 60 times a second. A frame interval setting of less than one results in undefined
    // behavior.
    if (framesPerSecond >= 1) {
        _framesPerSecond = framesPerSecond;

        if (_animating) {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation {
    if (!_animating) {
        // Create the display link and set the callback to our drawView method
        _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];

        // Set it to our _animationFrameInterval
        //_displayLink.preferredFramesPerSecond = _animationFrameInterval;
        _displayLink.preferredFramesPerSecond = _framesPerSecond;

        // Have the display link run on the default runn loop (and the main thread)
        [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

        _animating = TRUE;
    }
}

- (void)stopAnimation {
    if (_animating) {
        [_displayLink invalidate];
        _displayLink = nil;
        _animating = FALSE;
    }
}

- (void)dealloc {
    // tear down GL
    if (_defaultFBOName) {
        glDeleteFramebuffers(1, &_defaultFBOName);
        _defaultFBOName = 0;
    }

    if (_colorRenderbuffer) {
        glDeleteRenderbuffers(1, &_colorRenderbuffer);
        _colorRenderbuffer = 0;
    }

    // tear down context
    if ([EAGLContext currentContext] == _context)
        [EAGLContext setCurrentContext:nil];
    
    [super dealloc];
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesBegan:touches withEvent:event];
    float scaleFactor = self.contentScaleFactor;
    for (UITouch* touch in [touches allObjects]) {
        CGPoint location = [touch locationInView:self];
        float x = (float) (location.x * scaleFactor);
        float y = (float) (location.y * scaleFactor);
        g_app.dispatch({touch_event_type::begin, (uint64_t) touch, x, y});
    }
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesMoved:touches withEvent:event];
    float scaleFactor = self.contentScaleFactor;
    for (UITouch* touch in [touches allObjects]) {
        CGPoint location = [touch locationInView:self];
        float x = (float) (location.x * scaleFactor);
        float y = (float) (location.y * scaleFactor);
        g_app.dispatch({touch_event_type::move, (uint64_t) touch, x, y});
    }
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesEnded:touches withEvent:event];
    float scaleFactor = self.contentScaleFactor;
    for (UITouch* touch in [touches allObjects]) {
        CGPoint location = [touch locationInView:self];
        float x = (float) (location.x * scaleFactor);
        float y = (float) (location.y * scaleFactor);
        g_app.dispatch({touch_event_type::end, (uint64_t) touch, x, y});
    }
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesCancelled:touches withEvent:event];
    float scaleFactor = self.contentScaleFactor;
    for (UITouch* touch in [touches allObjects]) {
        CGPoint location = [touch locationInView:self];
        float x = (float) (location.x * scaleFactor);
        float y = (float) (location.y * scaleFactor);
        g_app.dispatch({touch_event_type::end, (uint64_t) touch, x, y});
    }
}

@end
