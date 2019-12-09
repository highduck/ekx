#import "mac_app_delegate.h"

#include <ek/util/logger.hpp>
#include <ek/app/app.hpp>

using namespace ek;

MacAppDelegate* g_app_delegate = nullptr;

@implementation MacAppDelegate

- (void)setupMenuBar {
    id menubar = [NSMenu new];
    id appMenuItem = [NSMenuItem new];
    [menubar addItem:appMenuItem];
    [_application setMainMenu:menubar];

    id appMenu = [NSMenu new];
    id quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit"
                                                 action:@selector(terminate:)
                                          keyEquivalent:@"q"];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
}

- (void)createView {
/*** init gl view ***/
    _view = [[MacOpenGLView alloc] init];
    g_app.view_context_ = (__bridge void*) _view;

    NSOpenGLPixelFormatAttribute attrs[] = {
//            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
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
    auto& config = g_app.creation_config;
    NSRect frame = NSMakeRect(100.0, 100.0, config.size.x, config.size.y);
    NSWindowStyleMask styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable |
                                  NSWindowStyleMaskMiniaturizable;
    NSRect rect = [NSWindow contentRectForFrameRect:frame styleMask:styleMask];
    _window = [[NSWindow alloc]
            initWithContentRect:rect
                      styleMask:styleMask
                        backing:NSBackingStoreBuffered
                          defer:NO];

    NSString* title = [NSString stringWithUTF8String:config.title.c_str()];
    [_window setTitle:title];

    _window.delegate = self;

    _window.acceptsMouseMovedEvents = YES;

    [_window setContentView:_view];
    [_window makeFirstResponder:_view];
    [_window makeKeyAndOrderFront:nil];
}

- (void)windowDidResize:(__unused NSNotification*)notification {
    NSRect frame = [NSWindow contentRectForFrameRect:_window.frame
                                           styleMask:_window.styleMask];
    EK_TRACE << "changed window_size (via windowDidResize)";
    g_app.window_size = {frame.size.width, frame.size.height};
    g_app.size_changed = true;
}

- (void)windowDidChangeBackingProperties:(__unused NSNotification*)notification {
    EK_TRACE("`windowDidChangeBackingProperties` changed device_pixel_ratio to %lf", _window.backingScaleFactor);
    g_app.content_scale = _window.backingScaleFactor;
    g_app.size_changed = true;
}

- (void)applicationWillFinishLaunching:(__unused NSNotification*)notification {
    _application = NSApplication.sharedApplication;
    [_application setActivationPolicy:NSApplicationActivationPolicyRegular];

    g_app.init();

    [self setupMenuBar];
    [self createView];
    [self createWindow];
}

- (void)applicationDidFinishLaunching:(__unused NSNotification*)notification {
    [_application activateIgnoringOtherApps:YES];
    g_app.start();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(__unused NSApplication*)theApplication {
    return YES;
}

- (void)applicationWillTerminate:(__unused NSNotification*)notification {
    g_app.dispatch({event_type::app_close});
}

- (void)applicationWillResignActive:(__unused NSNotification*)notification {
    g_app.dispatch({event_type::app_pause});
}

- (void)applicationDidBecomeActive:(__unused NSNotification*)notification {
    g_app.dispatch({event_type::app_resume});
}

@end
