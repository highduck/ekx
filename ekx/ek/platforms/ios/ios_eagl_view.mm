#import "ios_eagl_view.h"

#import <OpenGLES/ES2/gl.h>

#include <ek/app/app.hpp>
#include <apple_common.h>

using namespace ek::app;

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

void clear_buffers() {
    glDepthMask(GL_TRUE);
    glClearDepthf(1.0f);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);
}

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

    g_app.primary_frame_buffer = _defaultFBOName;

    clear_buffers();

    return self;
}

- (BOOL)resizeFromLayer:(CAEAGLLayer*)layer {
    // The pixel dimensions of the CAEAGLLayer
    GLint backing_width;
    GLint backing_height;

    // Allocate color buffer backing based on the current layer size
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backing_width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backing_height);

    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backing_width, backing_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);

    g_app.drawable_size.x = static_cast<float>(backing_width);
    g_app.drawable_size.y = static_cast<float>(backing_height);
    g_app.size_changed = true;

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return NO;
    }

    clear_buffers();

    return YES;
}


// Must return the CAEAGLLayer class so that CA allocates an EAGLLayer backing for this view
+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (bool)setupView {
    self.userInteractionEnabled = YES;
    self.multipleTouchEnabled = YES;

    // Get the layer
    CAEAGLLayer* glLayer = (CAEAGLLayer*) self.layer;

    glLayer.opaque = TRUE;
    glLayer.contentsScale = UIScreen.mainScreen.nativeScale;
    // kEAGLColorFormatSRGBA8: nice place to initialize sRGBA surface, but it's very depends on 2D/3D style
    // and require to be rendered from higher precision buffers of even Floating-point buffers, so currently
    // just leaved classic RGBA8
    glLayer.drawableProperties = @{
            kEAGLDrawablePropertyRetainedBacking: @NO,
            kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8
    };

    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (_context == nil) {
        _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        g_app.fallbackGLES2 = true;
    }

    if (!_context || ![EAGLContext setCurrentContext:_context]) {
        return false;
    }

    [self createRenderer:(id <EAGLDrawable>) self.layer];

    _animating = FALSE;
    _framesPerSecond = 60;
    _displayLink = nil;

    return true;
}

- (instancetype)initWithFrame:(CGRect)frame {
    if ((self = [super initWithFrame:frame])) {
        if ([self setupView]) {
            return self;
        } else {
            return nil;
        }
    }

    return self;
}

// The GL view is stored in the storyboard file. When it's unarchived it's sent -initWithCoder:
- (instancetype)initWithCoder:(NSCoder*)coder {
    if ((self = [super initWithCoder:coder])) {
        if ([self setupView]) {
            return self;
        } else {
            return nil;
        }
    }

    return self;
}

- (void)drawView:(id)sender {
    [EAGLContext setCurrentContext:_context];
    glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBOName);

    dispatch_draw_frame();

    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [_context presentRenderbuffer:GL_RENDERBUFFER];

    ek::apple::handle_exit_request();
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
    if (EAGLContext.currentContext == _context) {
        [EAGLContext setCurrentContext:nil];
    }
}

void handle_touches(event_type type, UIView* view, NSSet* touches, UIEvent* event) {
    event_t ev{type};
    const auto scale_factor = view.contentScaleFactor;
    for (UITouch* touch in [touches allObjects]) {
        const CGPoint location = [touch locationInView:view];
        ev.id = (uint64_t) touch;
        ev.pos.x = static_cast<float>(location.x * scale_factor);
        ev.pos.y = static_cast<float>(location.y * scale_factor);
        dispatch_event(ev);
    }
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesBegan:touches withEvent:event];
    handle_touches(event_type::touch_begin, self, touches, event);
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesMoved:touches withEvent:event];
    handle_touches(event_type::touch_move, self, touches, event);
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesEnded:touches withEvent:event];
    handle_touches(event_type::touch_end, self, touches, event);
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesCancelled:touches withEvent:event];
    handle_touches(event_type::touch_end, self, touches, event);
}

@end
