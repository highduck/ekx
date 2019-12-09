#include "mac_opengl_view.h"
#include "mac_utils.h"
#include <apple_common.h>

using namespace ek;

@implementation MacOpenGLView

//- (BOOL)wantsLayer {
//    return YES;
//}
//
//- (id)makeBackingLayer {
//    return [CALayer layer];
//}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)outputTime {
    (void) outputTime;
    // There is no autorelease pool when this method is called
    // because it will be called from a background thread.
    // It's important to create one or app can leak objects.
    @autoreleasepool {
        [self drawView];
    }
    return kCVReturnSuccess;
}

// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef, // displayLink
                                      const CVTimeStamp*,// now
                                      const CVTimeStamp* outputTime,
                                      CVOptionFlags, // flagsIn
                                      CVOptionFlags*, // flagsOut
                                      void* displayLinkContext) {
    CVReturn result = [(__bridge MacOpenGLView*) displayLinkContext getFrameForTime:outputTime];
    return result;
}

- (void)prepareOpenGL {
    [super prepareOpenGL];

    // Make all the OpenGL calls to setup rendering
    // and build the necessary rendering objects
    [self initGL];

    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);

    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, (__bridge void*) self);

    // Set the display link for the current renderer
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

    // Activate the display link
    CVDisplayLinkStart(displayLink);

    // Register to be notified when the window closes so we can stop the displaylink
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowWillClose:)
                                                 name:NSWindowWillCloseNotification
                                               object:self.window];
}

- (void)windowWillClose:(__unused NSNotification*)notification {
    // tricky check we are closing owner window
    // it could be FullscreenToolbar window when you exit MacOS default Maximized mode
    if (notification.object == self.window) {
        // Stop the display link when the window is closing because default
        // OpenGL render buffers will be destroyed.  If display link continues to
        // fire without renderbuffers, OpenGL draw calls will set errors.
        CVDisplayLinkStop(displayLink);
    }
}

- (void)initGL {
    // The reshape function may have changed the thread to which our OpenGL
    // context is attached before prepareOpenGL and initGL are called.  So call
    // makeCurrentContext to ensure that our OpenGL context current to this
    // thread (i.e. makeCurrentContext directs all OpenGL calls on this thread
    // to [self openGLContext])
    [[self openGLContext] makeCurrentContext];

    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLContextParameterSwapInterval];

    [self handleResize];
}

- (void)reshape {
    [super reshape];

    // We draw on a secondary thread through the display link. However, when
    // resizing the view, -drawRect is called on the main thread.
    // Add a mutex around to avoid the threads accessing the context
    // simultaneously when resizing.

    CGLLockContext([[self openGLContext] CGLContextObj]);

    //[[self openGLContext] update];

    [self handleResize];
    [self.openGLContext update];
//    CGLFlushDrawable([[self openGLContext] CGLContextObj]);//
    CGLUnlockContext([self.openGLContext CGLContextObj]);
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)handleResize {
    const auto scale = self.window.backingScaleFactor;
    const auto window_size = self.bounds.size;
    const auto drawable_size = [self convertRectToBacking:self.bounds].size;

    if (g_app.content_scale != scale ||
        g_app.window_size.x != window_size.width ||
        g_app.window_size.y != window_size.height ||
        g_app.drawable_size.x != drawable_size.width ||
        g_app.drawable_size.y != drawable_size.height) {
        g_app.content_scale = scale;
        g_app.window_size.x = window_size.width;
        g_app.window_size.y = window_size.height;
        g_app.drawable_size.x = drawable_size.width;
        g_app.drawable_size.y = drawable_size.height;
        g_app.size_changed = true;
    }
}

// Called whenever graphics state updated (such as window resize)
- (void)renewGState {
    // OpenGL rendering is not synchronous with other rendering on the OSX.
    // Therefore, call disableScreenUpdatesUntilFlush so the window server
    // doesn't render non-OpenGL content in the window asynchronously from
    // OpenGL content, which could cause flickering.  (non-OpenGL content
    // includes the title bar and draw2d done by the app with other APIs)
    [[self window] disableScreenUpdatesUntilFlush];

    [super renewGState];
}

// Called during resize operations
- (void)drawRect:(__unused NSRect)theRect {
    // Avoid flickering during resize by drawing
    [self drawView];
}

- (void)drawView {

//    [[self openGLContext] makeCurrentContext];
    // We draw on a secondary thread through the display link
    // When resizing the view, -reshape is called automatically on the main
    // thread. Add a mutex around to avoid the threads accessing the context
    // simultaneously when resizing
    CGLLockContext([[self openGLContext] CGLContextObj]);

    [[self openGLContext] makeCurrentContext];
    g_app.dispatch_draw_frame();

    CGLFlushDrawable([[self openGLContext] CGLContextObj]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);

    if (g_app.cursor_dirty) {
        g_app.cursor_dirty = false;
        [self.window performSelectorOnMainThread:@selector(invalidateCursorRectsForView:)
                                      withObject:self
                                   waitUntilDone:NO];
    }

    apple::handle_exit_request();
}

- (void)dealloc {
// Stop the display link BEFORE releasing anything in the view
// otherwise the display link thread may call into the view and crash
// when it encounters something that has been release
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
}

- (void)resetCursorRects {
    [super resetCursorRects];
    set_view_mouse_cursor(self);
}

/**** HANDLING MOUSE ****/

- (BOOL)isFlipped {
    return YES;
}

- (void)mouseDown:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)mouseUp:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)rightMouseDown:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)rightMouseUp:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)otherMouseDown:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)otherMouseUp:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)mouseMoved:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)mouseDragged:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)rightMouseDragged:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)otherMouseDragged:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)mouseEntered:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)mouseExited:(NSEvent*)event {
    handle_mouse_event(self, event);
}

- (void)scrollWheel:(NSEvent*)event {
    handle_mouse_event(self, event);
}

/**** HANDLE TOUCH *****/

//void handle_touch(event_type event_type, double scale_factor, NSTouch* touch) {
//    const CGPoint location = touch.normalizedPosition;
//    event_t ev{event_type};
//    ev.id = uint64_t(touch.identity) + 1u;
//    ev.set_position(location.x, location.y, scale_factor);
//    g_app.dispatch(ev);
//}
//
//- (void)touchesBeganWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseBegan inView: self];
//    for (NSTouch* touch in touches) {
//        handle_touch(event_type::touch_begin, self.window.backingScaleFactor, touch);
//    }
//}
//
//- (void)touchesMovedWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseMoved inView: self];
//    for (NSTouch* touch in touches) {
//        handle_touch(event_type::touch_move, self.window.backingScaleFactor, touch);
//    }
//}
//
//- (void)touchesEndedWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseEnded inView: self];
//    for (NSTouch* touch in touches) {
//        handle_touch(event_type::touch_end, self.window.backingScaleFactor, touch);
//    }
//}
//
//- (void)touchesCancelledWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseCancelled inView: self];
//    for (NSTouch* touch in touches) {
//        handle_touch(event_type::touch_end, self.window.backingScaleFactor, touch);
//    }
//}

/***** BLOCK GESTURES ****/
- (void)swipeWithEvent:(__unused NSEvent*)event {}

- (void)rotateWithEvent:(__unused NSEvent*)event {}

- (void)magnifyWithEvent:(__unused NSEvent*)event {}

void handle_key(NSEvent* event, event_type type) {
    event_t ev{type};
    ev.code = convert_key_code(event.keyCode);
    setup_modifiers(event.modifierFlags, ev);
    g_app.dispatch(ev);
}

- (void)keyDown:(NSEvent*)event {
    if (!event.ARepeat) {
        handle_key(event, event_type::key_down);
    }

    if (is_text_event(event)) {
        event_t ev{event_type::text};
        ev.characters = event.characters.UTF8String;
        g_app.dispatch(ev);
    }
}

- (void)keyUp:(NSEvent*)event {
    if (!event.ARepeat) {
        handle_key(event, event_type::key_up);
    }
}

- (void)flagsChanged:(NSEvent*)event {
    NSUInteger mask = convert_key_code_to_modifier_mask(event.keyCode);
    if (mask) {
        handle_key(event, (event.modifierFlags & mask)
                          ? event_type::key_down
                          : event_type::key_up);
    }
}

@end