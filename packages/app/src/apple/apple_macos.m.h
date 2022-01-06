#include <ek/app_native.h>
#include "apple_macos_util.h"

static void handleQuitRequest() {
    if (ek_app.state & EK_APP_STATE_EXIT_PENDING) {
        ek_app.state ^= EK_APP_STATE_EXIT_PENDING;
        [ek_app_delegate.window performClose:nil];
    }
}

@implementation AppDelegate

@synthesize application;
@synthesize window;
@synthesize view;

- (void)quitFromMenu:(id)sender {
    [[NSApplication sharedApplication] terminate:sender];
//    [_window performClose: nil];
}

- (void)setupMenuBar {
    log_debug("setup menu bar");
    NSMenu* menubar = [NSMenu new];
    NSMenuItem* menu_item = [NSMenuItem new];
    [menubar addItem:menu_item];

    // quit button
    NSMenu* menu = [NSMenu new];
    NSMenuItem* quit_menu_item = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                            action:@selector(quitFromMenu:)
                                                     keyEquivalent:@"q"];
    [menu addItem:quit_menu_item];
    [menu_item setSubmenu:menu];

    // set current app menu
    [self.application setMainMenu:menubar];
}

- (void)createView {
    log_debug("create view");
    self.view = [MetalView new];
    self.view.device = MTLCreateSystemDefaultDevice();

    [self.view updateTrackingAreas];
    self.view.preferredFramesPerSecond = 60 / ek_app.config.swap_interval;
    self.view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    self.view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    self.view.sampleCount = (NSUInteger) ek_app.config.sample_count;
    self.view.autoResizeDrawable = true;
    self.view.layer.magnificationFilter = kCAFilterNearest;

    self.view.allowedTouchTypes = NSTouchTypeMaskIndirect;
    [self.view setWantsRestingTouches:YES];
}

- (void)createWindow {
    log_debug("create window");
    ek_app_config* config = &ek_app.config;
    bool doCenter = true;
    const float w = config->window_width > 0 ? (float) config->window_width : config->width;
    const float h = config->window_height > 0 ? (float) config->window_height : config->height;
    doCenter = config->window_x != 0 || config->window_y != 0;
    NSRect frame = NSMakeRect(config->window_x, config->window_y, (double)w, (double)h);
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

    self.window = [[NSWindow alloc]
            initWithContentRect:rect
                      styleMask:styleMask
                        backing:NSBackingStoreBuffered
                          defer:NO];

    NSString* title = [NSString stringWithUTF8String:config->title];
    [self.window setTitle:title];

    self.window.delegate = self;
    self.window.acceptsMouseMovedEvents = YES;
    self.window.restorable = YES;

    [self.window setContentView:self.view];
    if (doCenter) {
        [self.window center];
    }
    [self.window makeFirstResponder:self.view];
    [self.window makeKeyAndOrderFront:nil];
    [self handleResize];
}

- (void)handleResize {
    const float scale = (float) self.window.backingScaleFactor;
    const CGSize drawableSize = [self.view convertRectToBacking:self.view.bounds].size;

    ek_app__update_viewport((ek_app_viewport) {
            .width = (float) drawableSize.width,
            .height = (float) drawableSize.height,
            .scale = scale,
            .insets = {0.0f, 0.0f, 0.0f, 0.0f}
    });
}

- (void)windowDidResize:(__unused NSNotification*)notification {
    [self handleResize];
}

- (void)windowDidChangeBackingProperties:(__unused NSNotification*)notification {
    [self handleResize];
}

- (void)applicationWillFinishLaunching:(__unused NSNotification*)notification {
    log_debug("applicationWillFinishLaunching canvas_begin");

    ek_app_delegate = self;

    self.application = NSApplication.sharedApplication;
    [self.application setActivationPolicy:NSApplicationActivationPolicyRegular];

    initScanCodeTableApple();
    ek_app__init();
    ek_app_main();
    if (ek_app.state & EK_APP_STATE_EXIT_PENDING) {
        exit(ek_app.exit_code);
    }

    [self setupMenuBar];
    [self createView];
    [self createWindow];
    log_debug("app macOS: applicationWillFinishLaunching end");
}

- (void)applicationDidFinishLaunching:(__unused NSNotification*)notification {
    log_debug("app macOS: applicationDidFinishLaunching canvas_begin");
    [self.application activateIgnoringOtherApps:YES];
    ek_app__notify_ready();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(__unused NSApplication*)theApplication {
    return YES;
}

- (void)applicationWillTerminate:(__unused NSNotification*)notification {
    ek_app__process_event((ek_app_event) {.type= EK_APP_EVENT_CLOSE});
    if (self.view != nil) {
        EK_OBJC_RELEASE(self.view.device)
    }
    EK_OBJC_RELEASE(self.view)
    //EKAPP_RELEASE(self.window);
}

- (void)applicationWillResignActive:(__unused NSNotification*)notification {
    ek_app__process_event((ek_app_event) {.type = EK_APP_EVENT_PAUSE});
}

- (void)applicationDidBecomeActive:(__unused NSNotification*)notification {
    ek_app__process_event((ek_app_event) {.type=EK_APP_EVENT_RESUME});
}

@end


@implementation MetalView

@synthesize defaultPass;

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
    setViewMouseCursor(self);
}

- (void)drawRect:(NSRect)rect {

    (void) rect;

    if (self.currentDrawable != nil && self.currentRenderPassDescriptor != nil) {
        // we need to keep ref for default render pass on current frame
        // it should be checked in RELEASE build to ensure all references are valid
        self.defaultPass = self.currentRenderPassDescriptor;
        ek_app__process_frame();
    }

    if (ek_app.state & EK_APP_STATE_CURSOR_CHANGED) {
        ek_app.state ^= EK_APP_STATE_CURSOR_CHANGED;
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
    handleMouseEvent(self, event);
}

- (void)mouseUp:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)rightMouseDown:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)rightMouseUp:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)otherMouseDown:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)otherMouseUp:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)mouseMoved:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)mouseDragged:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)rightMouseDragged:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)otherMouseDragged:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)mouseEntered:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)mouseExited:(NSEvent*)event {
    handleMouseEvent(self, event);
}

- (void)scrollWheel:(NSEvent*)event {
    handleWheelEvent(self, event);
}

/**** HANDLE TOUCH *****/

//void handle_touch(Event::Type event_type, double scale_factor, NSTouch* touch) {
//    const CGPoint location = touch.normalizedPosition;
//    Event ev{event_type};
//    ev.id = uint64_t(touch.identity) + 1u;
//    ev.set_position(location.x, location.y, scale_factor);
//    ek_app.dispatch(ev);
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

static void handleKeyEvent(NSEvent* event, bool keyDown) {
    const ek_app_event_type type = keyDown ? EK_APP_EVENT_KEY_DOWN : EK_APP_EVENT_KEY_UP;
    ek_app__process_event((ek_app_event) {.key = {
            .type = type,
            .code = toKeyCode(event.keyCode),
            .modifiers = toKeyModifiers(event.modifierFlags)
    }});
}

- (void)keyDown:(NSEvent*)event {

    if (!event.ARepeat) {
        handleKeyEvent(event, true);
    }

    if (isTextEvent(event)) {
        ek_app_event ev = {.type = EK_APP_EVENT_TEXT};
        strncpy(ev.text.data, event.characters.UTF8String, sizeof(ev.text.data));
        ek_app__process_event(ev);
    }
}

- (void)keyUp:(NSEvent*)event {
    if (!event.ARepeat) {
        handleKeyEvent(event, false);
    }
}

- (void)flagsChanged:(NSEvent*)event {
    NSUInteger mask = extractKeyModifiers(event.keyCode);
    if (mask) {
        handleKeyEvent(event, (event.modifierFlags & mask) != 0);
    }
}

@end
