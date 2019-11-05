#include "OpenGLView.h"

#include "osx_input.h"

#include <platform/application.hpp>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

using namespace ek;

@interface NSCursor (HelpCursor)
+ (NSCursor*)_helpCursor;
@end

@implementation OpenGLView

- (instancetype)init {
    [super init];

    return self;
}

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
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,
                                      const CVTimeStamp* now,
                                      const CVTimeStamp* outputTime,
                                      CVOptionFlags flagsIn,
                                      CVOptionFlags* flagsOut,
                                      void* displayLinkContext) {
    (void) displayLink; // unused
    (void) now; // unused
    (void) flagsIn; // unused
    (void) flagsOut; // unused

    CVReturn result = [(__bridge OpenGLView*) displayLinkContext getFrameForTime:outputTime];
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
                                               object:[self window]];
}

- (void)windowWillClose:(__unused NSNotification*)notification {
// Stop the display link when the window is closing because default
// OpenGL render buffers will be destroyed.  If display link continues to
// fire without renderbuffers, OpenGL draw calls will set errors.

    CVDisplayLinkStop(displayLink);
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
    //GLint swapInt = 0;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

    [self handleResize];

    // Init our renderer.  Use 0 for the defaultFBO which is appropriate for
    // OSX (but not iOS since iOS apps must create their own FBO)

    //_renderer = [[OpenGLRenderer alloc] initWithDefaultFBO:0];
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
    [[self openGLContext] update];
//    CGLFlushDrawable([[self openGLContext] CGLContextObj]);//
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)handleResize {
    auto& window = const_cast<window_t&>(g_window);

    window.device_pixel_ratio = static_cast<float>(self.window.backingScaleFactor);

    // Get the view size in Points
    NSRect viewRectPoints = [self bounds];
    window.window_size = {
            static_cast<uint32_t>(viewRectPoints.size.width),
            static_cast<uint32_t>(viewRectPoints.size.height)
    };

    // Rendering at retina resolutions will reduce aliasing, but at the potential
    // cost of framerate and battery life due to the GPU needing to render more
    // pixels.

    // Any calculations the renderer does which use pixel dimentions, must be
    // in "retina" space.  [NSView convertRectToBacking] converts point sizes
    // to pixel sizes.  Thus the renderer gets the size in pixels, not points,
    // so that it can set it's viewport and perform and other pixel based
    // calculations appropriately.
    // viewRectPixels will be larger than viewRectPoints for retina displays.
    // viewRectPixels will be the same as viewRectPoints for non-retina displays

    NSRect viewRectPixels = [self convertRectToBacking:[self bounds]];
    window.back_buffer_size = {
            static_cast<uint32_t>(viewRectPixels.size.width),
            static_cast<uint32_t>(viewRectPixels.size.height)
    };

    window.size_changed = true;
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
}

- (void)dealloc {
// Stop the display link BEFORE releasing anything in the view
// otherwise the display link thread may call into the view and crash
// when it encounters something that has been release
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
    [super dealloc];
}

- (void)resetCursorRects {
    [super resetCursorRects];
    NSCursor* cursor = nil;

    switch (g_window.cursor()) {
        case mouse_cursor_t::button:
            cursor = NSCursor.pointingHandCursor;
            break;
        case mouse_cursor_t::help:
            cursor = NSCursor._helpCursor;
            break;
        case mouse_cursor_t::arrow:
        case mouse_cursor_t::parent:
            cursor = NSCursor.arrowCursor;
            break;
    }

    if (cursor) {
        [self addCursorRect:[self bounds] cursor:cursor];
        [cursor set];
    }
}

/**** HANDLING MOUSE ****/

- (BOOL)isFlipped {
    return YES;
}

- (void)handleMouse:(NSEvent*)event {
    NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
    mouse_event_t ev{};
    auto scale = self.window.backingScaleFactor;
    ev.x = static_cast<float>(location.x * scale);
    ev.y = static_cast<float>(location.y * scale);

    switch (event.type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeLeftMouseUp:
            ev.button = mouse_button::left;
            break;
        case NSEventTypeRightMouseDown:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeRightMouseUp:
            ev.button = mouse_button::right;
            break;
        case NSEventTypeOtherMouseDown:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeOtherMouseUp:
            ev.button = mouse_button::other;
            break;
        default:
            break;
    }

    switch (event.type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:
            ev.type = mouse_event_type::down;
            break;
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:
            ev.type = mouse_event_type::up;
            break;
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeMouseMoved:
            ev.type = mouse_event_type::move;
            break;
        case NSEventTypeMouseEntered:
            ev.type = mouse_event_type::enter;
            break;
        case NSEventTypeMouseExited:
            ev.type = mouse_event_type::exit;
            break;
        case NSEventTypeScrollWheel:
            ev.type = mouse_event_type::scroll;
            osx_handle_mouse_wheel_scroll(event, ev);
            break;
        default:
            break;
    }
    g_app.dispatch(ev);
}

- (void)mouseDown:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)mouseUp:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)rightMouseDown:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)rightMouseUp:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)otherMouseDown:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)otherMouseUp:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)mouseMoved:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)mouseDragged:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)rightMouseDragged:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)otherMouseDragged:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)mouseEntered:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)mouseExited:(NSEvent*)event {
    [self handleMouse:event];
}

- (void)scrollWheel:(NSEvent*)event {
    [self handleMouse:event];
}

/**** HANDLE TOUCH *****/

void handleTouch(touch_event_type event_type, float scale_factor, NSTouch* touch) {
    CGPoint location = touch.normalizedPosition;
    touch_event_t ev{
            event_type,
            uint64_t([touch identity]) + 1u,
//            static_cast<uint64_t>(touch.identity) + 1u,
            static_cast<float>(scale_factor * location.x),
            static_cast<float>(scale_factor * location.y)
    };
    g_app.dispatch(ev);
}
//
//- (void)touchesBeganWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseBegan inView: self];
//    auto sc = (float)self.window.backingScaleFactor;
//    for (NSTouch* touch in touches) {
//        handleTouch(touch_event_type::begin, sc, touch);
//    }
//}
//
//- (void)touchesMovedWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseMoved inView: self];
//    auto sc = (float)self.window.backingScaleFactor;
//    for (NSTouch* touch in touches) {
//        handleTouch(touch_event_type::move, sc, touch);
//    }
//}
//
//- (void)touchesEndedWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseEnded inView: self];
//    auto sc = (float)self.window.backingScaleFactor;
//    for (NSTouch* touch in touches) {
//        handleTouch(touch_event_type::end, sc, touch);
//    }
//}
//
//- (void)touchesCancelledWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseCancelled inView: self];
//    auto sc = (float)self.window.backingScaleFactor;
//    for (NSTouch* touch in touches) {
//        handleTouch(touch_event_type::end, sc, touch);
//    }
//}

/***** BLOCK GESTURES ****/
- (void)swipeWithEvent:(__unused NSEvent*)event {}

- (void)rotateWithEvent:(__unused NSEvent*)event {}

- (void)magnifyWithEvent:(__unused NSEvent*)event {}


- (void)keyDown:(NSEvent*)event {
    auto key_code = convert_key_code(event.keyCode);
    if (!event.ARepeat) {
        key_event_t key{key_event_type::down, key_code};
        setup_modifiers(event.modifierFlags, key);
        g_app.dispatch(key);
    }
    if (is_text_event(event)) {
        g_app.dispatch(text_event_t{event.characters.UTF8String});
    }
}

- (void)keyUp:(NSEvent*)event {
    auto key_code = convert_key_code(event.keyCode);
    if (!event.ARepeat) {
        key_event_t key{key_event_type::up, key_code};
        setup_modifiers(event.modifierFlags, key);
        g_app.dispatch(key);
    }
}

- (void)flagsChanged:(NSEvent*)event {
    NSUInteger mask = convert_key_code_to_modifier_mask(event.keyCode);
    if (mask) {
        key_event_t key{(event.modifierFlags & mask)
                        ? key_event_type::down
                        : key_event_type::up,
                        convert_key_code(event.keyCode)};
        setup_modifiers(event.modifierFlags, key);
        g_app.dispatch(key);
    }
}

@end