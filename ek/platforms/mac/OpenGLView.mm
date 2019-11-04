#include "OpenGLView.h"

#include "platform/Application.h"
#include "platform/Window.h"
#include "osx_input.h"

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

using TouchEvent = ek::TouchEvent;
using KeyEvent = ek::KeyEvent;
using MouseEvent = ek::MouseEvent;
using MouseCursor = ek::MouseCursor;

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

- (CVReturn)getFrameForTime: (const CVTimeStamp*)outputTime {
    (void)outputTime;
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
    (void)displayLink; // unused
    (void)now; // unused
    (void)flagsIn; // unused
    (void)flagsOut; // unused

    CVReturn result = [(__bridge OpenGLView*)displayLinkContext getFrameForTime: outputTime];
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
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, (__bridge void*)self);

    // Set the display link for the current renderer
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

    // Activate the display link
    CVDisplayLinkStart(displayLink);

    // Register to be notified when the window closes so we can stop the displaylink
    [[NSNotificationCenter defaultCenter] addObserver: self
                                             selector: @selector(windowWillClose:)
                                                 name: NSWindowWillCloseNotification
                                               object: [self window]];
}

- (void)windowWillClose: (__unused NSNotification*)notification {
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
    [[self openGLContext] setValues: &swapInt forParameter: NSOpenGLCPSwapInterval];

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
    auto& window = const_cast<ek::Window&>(ek::gWindow);

    window.scaleFactor = static_cast<float>(self.window.backingScaleFactor);

    // Get the view size in Points
    NSRect viewRectPoints = [self bounds];
    window.windowSize = {
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

    NSRect viewRectPixels = [self convertRectToBacking: [self bounds]];
    window.backBufferSize = {
            static_cast<uint32_t>(viewRectPixels.size.width),
            static_cast<uint32_t>(viewRectPixels.size.height)
    };

    window.sizeChanged = true;
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
- (void)drawRect: (__unused NSRect)theRect {
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
    ek::gApp.dispatchDrawFrame();

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

    switch (ek::gWindow.cursor()) {
        case MouseCursor::Button :
            cursor = NSCursor.pointingHandCursor;
            break;
        case MouseCursor::Help :
            cursor = NSCursor._helpCursor;
            break;
        default:
            cursor = NSCursor.arrowCursor;
            break;
    }

    if (cursor) {
        [self addCursorRect: [self bounds] cursor: cursor];
        [cursor set];
    }
}

/**** HANDLING MOUSE ****/

- (BOOL)isFlipped {
    return YES;
}

- (void)handleMouse: (NSEvent*)event {
    NSPoint location = [self convertPoint: event.locationInWindow fromView: nil];
    MouseEvent ev{};
    auto scale = self.window.backingScaleFactor;
    ev.x = static_cast<float>(location.x * scale);
    ev.y = static_cast<float>(location.y * scale);

    switch (event.type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeLeftMouseUp:
            ev.button = MouseEvent::Button::Left;
            break;
        case NSEventTypeRightMouseDown:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeRightMouseUp:
            ev.button = MouseEvent::Button::Right;
            break;
        case NSEventTypeOtherMouseDown:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeOtherMouseUp:
            ev.button = MouseEvent::Button::Other;
            break;
        default:
            break;
    }

    switch (event.type) {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:
            ev.type = MouseEvent::Type::Down;
            break;
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:
            ev.type = MouseEvent::Type::Up;
            break;
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged:
        case NSEventTypeMouseMoved:
            ev.type = MouseEvent::Type::Move;
            break;
        case NSEventTypeMouseEntered:
            ev.type = MouseEvent::Type::Enter;
            break;
        case NSEventTypeMouseExited:
            ev.type = MouseEvent::Type::Exit;
            break;
        case NSEventTypeScrollWheel:
            ev.type = MouseEvent::Type::Scroll;
            ek::osx_handle_mouse_wheel_scroll(event, ev);
            break;
        default:
            break;
    }
    ek::gApp.dispatch(ev);
}

- (void)mouseDown: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)mouseUp: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)rightMouseDown: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)rightMouseUp: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)otherMouseDown: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)otherMouseUp: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)mouseMoved: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)mouseDragged: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)rightMouseDragged: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)otherMouseDragged: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)mouseEntered: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)mouseExited: (NSEvent*)event {
    [self handleMouse: event];
}

- (void)scrollWheel: (NSEvent*)event {
    [self handleMouse: event];
}

/**** HANDLE TOUCH *****/

void handleTouch(TouchEvent::Type eventType, float scaleFactor, NSTouch* touch) {
    CGPoint location = touch.normalizedPosition;
    TouchEvent ev{
            eventType,
            uint64_t([touch identity]) + 1u,
//            static_cast<uint64_t>(touch.identity) + 1u,
            static_cast<float>(scaleFactor * location.x),
            static_cast<float>(scaleFactor * location.y)
    };
    ek::gApp.dispatch(ev);
}
//
//- (void)touchesBeganWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseBegan inView: self];
//    auto sc = (float)self.window.backingScaleFactor;
//    for (NSTouch* touch in touches) {
//        handleTouch(TouchEvent::Type::Begin, sc, touch);
//    }
//}
//
//- (void)touchesMovedWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseMoved inView: self];
//    auto sc = (float)self.window.backingScaleFactor;
//    for (NSTouch* touch in touches) {
//        handleTouch(TouchEvent::Type::Move, sc, touch);
//    }
//}
//
//- (void)touchesEndedWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseEnded inView: self];
//    auto sc = (float)self.window.backingScaleFactor;
//    for (NSTouch* touch in touches) {
//        handleTouch(TouchEvent::Type::End, sc, touch);
//    }
//}
//
//- (void)touchesCancelledWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseCancelled inView: self];
//    auto sc = (float)self.window.backingScaleFactor;
//    for (NSTouch* touch in touches) {
//        handleTouch(TouchEvent::Type::End, sc, touch);
//    }
//}

/***** BLOCK GESTURES ****/
- (void)swipeWithEvent: (__unused NSEvent*)event {}

- (void)rotateWithEvent: (__unused NSEvent*)event {}

- (void)magnifyWithEvent: (__unused NSEvent*)event {}


- (void)keyDown: (NSEvent*)event {
    auto key_code = ek::convert_key_code(event.keyCode);
    if (!event.ARepeat) {
        KeyEvent key{KeyEvent::Type::Down, key_code};
        setup_modifiers(event.modifierFlags, key);
        ek::gApp.dispatch(key);
    }
    if (ek::is_text_event(event)) {
        ek::gApp.dispatch(ek::text_event_t{event.characters.UTF8String});
    }
}

- (void)keyUp: (NSEvent*)event {
    auto key_code = ek::convert_key_code(event.keyCode);
    if (!event.ARepeat) {
        KeyEvent key{KeyEvent::Type::Up, key_code};
        setup_modifiers(event.modifierFlags, key);
        ek::gApp.dispatch(key);
    }
}

- (void)flagsChanged: (NSEvent*)event {
    NSUInteger mask = ek::convert_key_code_to_modifier_mask(event.keyCode);
    if (mask) {
        KeyEvent key{(event.modifierFlags & mask)
                     ? KeyEvent::Type::Down
                     : KeyEvent::Type::Up,
                     ek::convert_key_code(event.keyCode)};
        setup_modifiers(event.modifierFlags, key);
        ek::gApp.dispatch(key);
    }
}

@end