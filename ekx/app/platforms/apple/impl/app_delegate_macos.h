#pragma once

#import "app_util_macos.h"

#include <ek/app/Platform.h>

void handleQuitRequest() {
    using ek::app::g_app;
    if (g_app.exitRequired) {
        g_app.exitRequired = false;
        [gAppDelegate.window performClose:nil];
    }
}

@implementation AppDelegate

- (void)quitFromMenu:(id)sender {
    [[NSApplication sharedApplication] terminate:sender];
//    [_window performClose: nil];
}

- (void)setupMenuBar {
    EKAPP_LOG("setup menu bar");
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
    using namespace ek::app;
    EKAPP_LOG("create view");
    _view = [MetalView new];
    _view.device = MTLCreateSystemDefaultDevice();

    [_view updateTrackingAreas];
    _view.preferredFramesPerSecond = 60 / g_app.config.swapInterval;
    _view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    _view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    _view.sampleCount = (NSUInteger) g_app.config.sampleCount;
    _view.autoResizeDrawable = true;
    _view.layer.magnificationFilter = kCAFilterNearest;

    _view.allowedTouchTypes = NSTouchTypeMaskIndirect;
    [_view setWantsRestingTouches:YES];
}

- (void)createWindow {
    using namespace ek::app;
    EKAPP_LOG("create window");
    auto& config = g_app.config;
    bool doCenter = true;
    NSRect frame = NSMakeRect(0.0, 0.0, config.width, config.height);
    {
        const char* wndSettings = findArgumentValue("--window", nullptr);
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
    using namespace ek::app;

    const auto scale = static_cast<float>(_window.backingScaleFactor);
    const auto windowSize = _view.bounds.size;
    const auto drawableSize = [_view convertRectToBacking:_view.bounds].size;
    const auto windowWidth = static_cast<float>(windowSize.width);
    const auto windowHeight = static_cast<float>(windowSize.height);
    const auto backingWidth = static_cast<float>(drawableSize.width);
    const auto backingHeight = static_cast<float>(drawableSize.height);

    if (g_app.dpiScale != scale ||
        g_app.windowWidth != windowWidth ||
        g_app.windowHeight != windowHeight ||
        g_app.drawableWidth != backingWidth ||
        g_app.drawableHeight != backingHeight) {

        g_app.dpiScale = scale;
        g_app.windowWidth = windowWidth;
        g_app.windowHeight = windowHeight;
        g_app.drawableWidth = backingWidth;
        g_app.drawableHeight = backingHeight;

        g_app.dirtySize = true;
    }
}

- (void)windowDidResize:(__unused NSNotification*)notification {
    [self handleResize];
}

- (void)windowDidChangeBackingProperties:(__unused NSNotification*)notification {
    [self handleResize];
}

- (void)applicationWillFinishLaunching:(__unused NSNotification*)notification {
    EKAPP_LOG("applicationWillFinishLaunching begin");

    gAppDelegate = self;

    _application = NSApplication.sharedApplication;
    [_application setActivationPolicy:NSApplicationActivationPolicyRegular];

    ek::app::initScanCodeTableApple();
    ek::app::main();

    [self setupMenuBar];
    [self createView];
    [self createWindow];
    EKAPP_LOG("app macOS: applicationWillFinishLaunching end");
}

- (void)applicationDidFinishLaunching:(__unused NSNotification*)notification {
    using namespace ek::app;
    EKAPP_LOG("app macOS: applicationDidFinishLaunching begin");
    [_application activateIgnoringOtherApps:YES];
    notifyReady();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(__unused NSApplication*)theApplication {
    return YES;
}

- (void)applicationWillTerminate:(__unused NSNotification*)notification {
    using ek::app::EventType;
    using namespace ek::app;
    processEvent(EventType::Close);
    if (_view != nil) {
        EKAPP_RELEASE(_view.device);
    }
    EKAPP_RELEASE(_view);
    //EKAPP_RELEASE(_window);
}

- (void)applicationWillResignActive:(__unused NSNotification*)notification {
    using ek::app::EventType;
    using namespace ek::app;
    processEvent(EventType::Pause);
}

- (void)applicationDidBecomeActive:(__unused NSNotification*)notification {
    using ek::app::EventType;
    using namespace ek::app;
    processEvent(EventType::Resume);
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
    ek::app::setViewMouseCursor(self);
}

- (void)drawRect:(NSRect)rect {
    using namespace ek::app;
    (void) rect;

    if (self.currentDrawable != nil && self.currentRenderPassDescriptor != nil) {
        // we need to keep ref for default render pass on current frame
        // it should be checked in RELEASE build to ensure all references are valid
        self.defaultPass = self.currentRenderPassDescriptor;
        processFrame();
    }

    if (g_app.dirtyCursor) {
        g_app.dirtyCursor = false;
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
    ek::app::handleMouseEvent(self, event);
}

- (void)mouseUp:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)rightMouseDown:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)rightMouseUp:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)otherMouseDown:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)otherMouseUp:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)mouseMoved:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)mouseDragged:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)rightMouseDragged:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)otherMouseDragged:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)mouseEntered:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)mouseExited:(NSEvent*)event {
    ek::app::handleMouseEvent(self, event);
}

- (void)scrollWheel:(NSEvent*)event {
    ek::app::handleWheelEvent(self, event);
}

/**** HANDLE TOUCH *****/

//void handle_touch(Event::Type event_type, double scale_factor, NSTouch* touch) {
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

void handleKeyEvent(NSEvent* event, bool keyDown) {
    using namespace ek::app;
    using ek::app::EventType;
    const auto type = keyDown ? EventType::KeyDown : EventType::KeyUp;
    processEvent(KeyEvent{
            type,
            toKeyCode(event.keyCode),
            toKeyModifiers(event.modifierFlags)
    });
}

- (void)keyDown:(NSEvent*)event {
    using ek::app::EventType;
    using namespace ek::app;

    if (!event.ARepeat) {
        handleKeyEvent(event, true);
    }

    if (isTextEvent(event)) {
        processEvent(TextEvent{event.characters.UTF8String});
    }
}

- (void)keyUp:(NSEvent*)event {
    if (!event.ARepeat) {
        handleKeyEvent(event, false);
    }
}

- (void)flagsChanged:(NSEvent*)event {
    using ek::app::EventType;
    using namespace ek::app;

    NSUInteger mask = extractKeyModifiers(event.keyCode);
    if (mask) {
        handleKeyEvent(event, !!(event.modifierFlags & mask));
    }
}

@end
