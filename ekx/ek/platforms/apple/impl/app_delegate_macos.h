#pragma once

#import "../app_apple.h"
#import "app_util_macos.h"

#include <ek/app/app.hpp>

using namespace ek;
using namespace ek::app;

void handleQuitRequest() {
    if (g_app.require_exit) {
        g_app.require_exit = false;
        [gAppDelegate.window performClose:nil];
    }
}

@implementation AppDelegate

- (void)quitFromMenu:(id)sender {
    [[NSApplication sharedApplication] terminate:sender];
//    [_window performClose: nil];
}

- (void)setupMenuBar {
    EK_TRACE << "app macOS: setup menu bar";
    id menubar = [NSMenu new];
    id menu_item = [NSMenuItem new];
    [menubar addItem:menu_item];

    // quit button
    id menu = [NSMenu new];
    id quit_menu_item = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                   action:@selector(quitFromMenu:)
                                            keyEquivalent:@"q"];
    [menu addItem:quit_menu_item];
    [menu_item setSubmenu:menu];

    // set current app menu
    [_application setMainMenu:menubar];
}

- (void)createView {
    EK_TRACE << "app macOS: create view";
    _view = [MetalView new];
    _view.device = MTLCreateSystemDefaultDevice();

    [_view updateTrackingAreas];
    _view.preferredFramesPerSecond = 60 / g_app.window_cfg.swapInterval;
    _view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    _view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    _view.sampleCount = (NSUInteger) g_app.window_cfg.sampleCount;
    _view.autoResizeDrawable = true;
    _view.layer.magnificationFilter = kCAFilterNearest;

    _view.allowedTouchTypes = NSTouchTypeMaskIndirect;
    [_view setWantsRestingTouches:YES];
}

- (void)createWindow {
    EK_TRACE << "app macOS: create window";
    auto& config = g_app.window_cfg;
    bool doCenter = true;
    NSRect frame = NSMakeRect(0.0, 0.0, config.size.x, config.size.y);
    {
        const char* wndSettings = Arguments::current.getValue("--window", nullptr);
        if (wndSettings != nullptr) {
            sscanf(wndSettings, "%lf,%lf,%lf,%lf",
                   &frame.origin.x, &frame.origin.y,
                   &frame.size.width, &frame.size.height);
            doCenter = false;
        }
    }
    if (!doCenter) {
        // Window bounds (x, y, width, height)
        NSRect screenRect = [[NSScreen mainScreen] frame];
        //CGFloat scaleFactor = [[NSScreen mainScreen] backingScaleFactor];
        frame = NSMakeRect(frame.origin.x,
                           screenRect.size.height - frame.origin.y - frame.size.height,
                           frame.size.width,
                           frame.size.height);
    }

    NSWindowStyleMask styleMask = NSWindowStyleMaskTitled |
                                  NSWindowStyleMaskClosable |
                                  NSWindowStyleMaskResizable |
                                  NSWindowStyleMaskMiniaturizable;
    NSRect rect = [NSWindow contentRectForFrameRect:frame styleMask:styleMask];

    _window = [[NSWindow alloc]
            initWithContentRect:rect
                      styleMask:styleMask
                        backing:NSBackingStoreBuffered
                          defer:NO];

    [_window setTitle:[NSString stringWithUTF8String:config.title]];

    _window.delegate = self;
    _window.acceptsMouseMovedEvents = YES;
    _window.restorable = YES;

    [_window setContentView:_view];
    if (doCenter) {
        [_window center];
    }
    [_window makeFirstResponder:_view];
    [_window makeKeyAndOrderFront:nil];
    [self handleResize];
}

- (void)handleResize {
    const auto scale = static_cast<float>(_window.backingScaleFactor);
    const auto windowSize = _view.bounds.size;
    const auto drawableSize = [_view convertRectToBacking:_view.bounds].size;
    const auto windowWidth = static_cast<float>(windowSize.width);
    const auto windowHeight = static_cast<float>(windowSize.height);
    const auto backingWidth = static_cast<float>(drawableSize.width);
    const auto backingHeight = static_cast<float>(drawableSize.height);

    if (g_app.content_scale != scale ||
        g_app.window_size.x != windowWidth ||
        g_app.window_size.y != windowHeight ||
        g_app.drawable_size.x != backingWidth ||
        g_app.drawable_size.y != backingHeight) {

        g_app.content_scale = scale;
        g_app.window_size.x = windowWidth;
        g_app.window_size.y = windowHeight;
        g_app.drawable_size.x = backingWidth;
        g_app.drawable_size.y = backingHeight;

        g_app.size_changed = true;
    }
}

- (void)windowDidResize:(__unused NSNotification*)notification {
    [self handleResize];
}

- (void)windowDidChangeBackingProperties:(__unused NSNotification*)notification {
    [self handleResize];
}

- (void)applicationWillFinishLaunching:(__unused NSNotification*)notification {
    EK_TRACE << "app macOS: applicationWillFinishLaunching begin";

    gAppDelegate = self;

    _application = NSApplication.sharedApplication;
    [_application setActivationPolicy:NSApplicationActivationPolicyRegular];

    macos_init_common();

    dispatch_init();

    [self setupMenuBar];
    [self createView];
    [self createWindow];
    EK_TRACE << "app macOS: applicationWillFinishLaunching end";
}

- (void)applicationDidFinishLaunching:(__unused NSNotification*)notification {
    EK_TRACE << "app macOS: applicationDidFinishLaunching begin";
    [_application activateIgnoringOtherApps:YES];
    dispatch_device_ready();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(__unused NSApplication*)theApplication {
    return YES;
}

- (void)applicationWillTerminate:(__unused NSNotification*)notification {
    process_event({Event::Close});
    if (_view != nil) {
        OBJC_RELEASE(_view.device);
    }
    OBJC_RELEASE(_view);
    //OBJC_RELEASE(_window);
}

- (void)applicationWillResignActive:(__unused NSNotification*)notification {
    dispatch_event({Event::Pause});
}

- (void)applicationDidBecomeActive:(__unused NSNotification*)notification {
    dispatch_event({Event::Resume});
}

@end


@implementation MetalView

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)isOpaque {
    return YES;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (void)resetCursorRects {
    [super resetCursorRects];
    set_view_mouse_cursor(self);
}

- (void)drawRect:(NSRect)rect {
    (void) rect;

    if (self.currentDrawable != nil && self.currentRenderPassDescriptor != nil) {
        // we need to keep ref for default render pass on current frame
        // it should be checked in RELEASE build to ensure all references are valid
        self.defaultPass = self.currentRenderPassDescriptor;
        dispatch_draw_frame();
    }

    if (g_app.cursor_dirty) {
        g_app.cursor_dirty = false;
        [self.window performSelectorOnMainThread:@selector(invalidateCursorRectsForView:)
                                      withObject:self
                                   waitUntilDone:NO];
    }

    handleQuitRequest();
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

//void handle_touch(AppEventType event_type, double scale_factor, NSTouch* touch) {
//    const CGPoint location = touch.normalizedPosition;
//    Event ev{event_type};
//    ev.id = uint64_t(touch.identity) + 1u;
//    ev.set_position(location.x, location.y, scale_factor);
//    g_app.dispatch(ev);
//}
//
//- (void)touchesBeganWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseBegan inView: self];
//    for (NSTouch* touch in touches) {
//        handle_touch(Event::TouchBegin, self.window.backingScaleFactor, touch);
//    }
//}
//
//- (void)touchesMovedWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseMoved inView: self];
//    for (NSTouch* touch in touches) {
//        handle_touch(Event::TouchMove, self.window.backingScaleFactor, touch);
//    }
//}
//
//- (void)touchesEndedWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseEnded inView: self];
//    for (NSTouch* touch in touches) {
//        handle_touch(Event::TouchEnd, self.window.backingScaleFactor, touch);
//    }
//}
//
//- (void)touchesCancelledWithEvent: (NSEvent*)event {
//    NSSet* touches = [event touchesMatchingPhase: NSTouchPhaseCancelled inView: self];
//    for (NSTouch* touch in touches) {
//        handle_touch(Event::TouchEnd, self.window.backingScaleFactor, touch);
//    }
//}

/***** BLOCK GESTURES ****/
- (void)swipeWithEvent:(__unused NSEvent*)event {}

- (void)rotateWithEvent:(__unused NSEvent*)event {}

- (void)magnifyWithEvent:(__unused NSEvent*)event {}

void handle_key(NSEvent* event, Event::Type type) {
    Event ev{type};
    ev.keyCode = convert_key_code(event.keyCode);
    setup_modifiers(event.modifierFlags, ev);
    dispatch_event(ev);
}

- (void)keyDown:(NSEvent*)event {
    if (!event.ARepeat) {
        handle_key(event, Event::KeyDown);
    }

    if (is_text_event(event)) {
        Event ev{Event::Text};
        ev.characters = event.characters.UTF8String;
        dispatch_event(ev);
    }
}

- (void)keyUp:(NSEvent*)event {
    if (!event.ARepeat) {
        handle_key(event, Event::KeyUp);
    }
}

- (void)flagsChanged:(NSEvent*)event {
    NSUInteger mask = convert_key_code_to_modifier_mask(event.keyCode);
    if (mask) {
        handle_key(event, (event.modifierFlags & mask) ? Event::KeyDown : Event::KeyUp);
    }
}

@end
