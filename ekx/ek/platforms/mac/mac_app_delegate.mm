#import "mac_app_delegate.h"
#import "mac_utils.h"

#include <ek/util/logger.hpp>

using namespace ek;
using namespace ek::app;

@implementation MacAppDelegate

- (void)quitFromMenu:(id)sender {
    [[NSApplication sharedApplication] terminate:sender];
}

- (void)setupMenuBar {
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
    _view = [MacOpenGLView new];
    g_app.view_context_ = (__bridge void*) _view;

    NSOpenGLPixelFormatAttribute attrs[] = {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
//            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADepthSize, 32,
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFANoRecovery,
            0
    };

    NSOpenGLPixelFormat* pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];

#ifndef NDEBUG
    // When we're using a CoreProfile context, crash if we call a legacy OpenGL function
    // This will make it much more obvious where and when such a function call is made so
    // that we can remove such calls.
    // Without this we'd simply get GL_INVALID_OPERATION error for calling legacy functions
    // but it would be more difficult to see where that function was called.
    CGLEnable([context CGLContextObj], kCGLCECrashOnRemovedFunctions);
#endif

    _view.allowedTouchTypes = NSTouchTypeMaskIndirect;
    [_view setWantsRestingTouches:YES];

    [_view setPixelFormat:pf];
    [_view setOpenGLContext:context];
    [_view setWantsBestResolutionOpenGLSurface:YES];
}

- (void)createWindow {
    auto& config = g_app.window_cfg;
    NSRect frame = NSMakeRect(100.0, 100.0, config.size.x, config.size.y);
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

    [_window setTitle:[NSString stringWithUTF8String:config.title.c_str()]];

    _window.delegate = self;
    _window.acceptsMouseMovedEvents = YES;

    [_window setContentView:_view];
    [_window makeFirstResponder:_view];
    [_window makeKeyAndOrderFront:nil];
}

- (void)windowDidResize:(__unused NSNotification*)notification {
    const NSSize size = [NSWindow contentRectForFrameRect:_window.frame
                                          styleMask:_window.styleMask].size;
    //EK_TRACE("[macOS GL] windowDidResize: %lf, %lf", size.width, size.height);

    if (size.width != g_app.window_size.x || size.height != g_app.window_size.y) {
        g_app.window_size.x = static_cast<float>(size.width);
        g_app.window_size.y = static_cast<float>(size.height);
        g_app.size_changed = true;
        EK_TRACE("[macOS GL] windowDidResize APPLIED");
    }
}

- (void)windowDidChangeBackingProperties:(__unused NSNotification*)notification {
    EK_TRACE("[macOS GL] windowDidChangeBackingProperties: %lf", self.window.backingScaleFactor);
    const auto scale = static_cast<float>(_window.backingScaleFactor);
    if (g_app.content_scale != scale) {
        EK_TRACE("[macOS GL] windowDidChangeBackingProperties APPLIED");
        g_app.content_scale = scale;
        g_app.size_changed = true;
    }
}

- (void)applicationWillFinishLaunching:(__unused NSNotification*)notification {
    EK_TRACE("[macOS GL] applicationWillFinishLaunching");

    _application = NSApplication.sharedApplication;
    [_application setActivationPolicy:NSApplicationActivationPolicyRegular];

    macos_init_common();

    dispatch_init();

    [self setupMenuBar];
    [self createView];
    [self createWindow];
}

- (void)applicationDidFinishLaunching:(__unused NSNotification*)notification {
    [_application activateIgnoringOtherApps:YES];
    [[[self view] openGLContext] makeCurrentContext];
    dispatch_device_ready();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(__unused NSApplication*)theApplication {
    return YES;
}

- (void)applicationWillTerminate:(__unused NSNotification*)notification {
    dispatch_event({event_type::app_close});
}

- (void)applicationWillResignActive:(__unused NSNotification*)notification {
    dispatch_event({event_type::app_pause});
}

- (void)applicationDidBecomeActive:(__unused NSNotification*)notification {
    dispatch_event({event_type::app_resume});
}

@end
