#import "AppDelegate.h"

#include <ek/logger.hpp>
#include "platform/application.hpp"
#include "platform/window.hpp"

@implementation AppDelegate

- (void)setupMenuBar {
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];

    id appMenu = [[NSMenu new] autorelease];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Quit"
                                                  action:@selector(terminate:)
                                           keyEquivalent:@"q"]
            autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
}

- (void)dealloc {
    [openGLView release];
    [nsWindow release];
    [super dealloc];
}

- (void)createView {
/*** init gl view ***/
    OpenGLView* view = [[OpenGLView alloc] init];
    openGLView = view;

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

    view.allowedTouchTypes = NSTouchTypeMaskIndirect;
    [view setWantsRestingTouches:YES];

    [view setPixelFormat:pf];
    [view setOpenGLContext:context];
    [view setWantsBestResolutionOpenGLSurface:YES];
}

- (void)createWindow {
    auto& config = ek::g_window.creation_config;
    NSRect frame = NSMakeRect(100, 100, config.width, config.height);
    NSWindowStyleMask styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable |
                                  NSWindowStyleMaskMiniaturizable;
    NSRect rect = [NSWindow contentRectForFrameRect:frame styleMask:styleMask];
    NSWindow* wnd = [[NSWindow alloc]
            initWithContentRect:rect
                      styleMask:styleMask
                        backing:NSBackingStoreBuffered
                          defer:NO];
    nsWindow = wnd;

    NSString* title = [NSString stringWithUTF8String:config.title.c_str()];
    [wnd setTitle:title];

    wnd.delegate = self;

    wnd.acceptsMouseMovedEvents = YES;

    [wnd setContentView:openGLView];
    [wnd makeFirstResponder:openGLView];
    [wnd makeKeyAndOrderFront:nil];
}

- (void)windowDidResize:(__unused NSNotification*)notification {
    NSRect frame = [NSWindow contentRectForFrameRect:nsWindow.frame
                                           styleMask:nsWindow.styleMask];
    EK_DEBUG << "changed window_size (via windowDidResize)";
    ek::g_window.window_size = {
            static_cast<uint32_t>(frame.size.width),
            static_cast<uint32_t>(frame.size.height)
    };
    ek::g_window.size_changed = true;
}

- (void)windowDidChangeBackingProperties:(__unused NSNotification*)notification {
    EK_DEBUG << "`windowDidChangeBackingProperties` changed device_pixel_ratio to " << nsWindow.backingScaleFactor;
    ek::g_window.device_pixel_ratio = static_cast<float>(nsWindow.backingScaleFactor);
    ek::g_window.size_changed = true;
}

- (void)applicationWillFinishLaunching:(__unused NSNotification*)notification {
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    ek::g_app.init();

    [self setupMenuBar];
    [self createView];
    [self createWindow];
}

- (void)applicationDidFinishLaunching:(__unused NSNotification*)notification {
    [NSApp activateIgnoringOtherApps:YES];
    ek::g_app.start();
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(__unused NSApplication*)theApplication {
    return YES;
}

- (void)applicationWillTerminate:(__unused NSNotification*)notification {
    ek::g_app.dispatch({ek::app_event_type::close});
}

- (void)applicationWillResignActive:(__unused NSNotification*)notification {
    ek::g_app.dispatch({ek::app_event_type::paused});
}

- (void)applicationDidBecomeActive:(__unused NSNotification*)notification {
    ek::g_app.dispatch({ek::app_event_type::resumed});
}

@end
