#pragma once

#import <UIKit/UIKit.h>
#import "../app_apple.h"

#include <ek/app/app.hpp>

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    gAppDelegate = self;

// maybe if you have got this => disable new iOS 13.0 Scene Manifest behavior in Info.plist
//assert(self.window != nil);

    if (!self.window) {
        self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
        self.window.rootViewController = [UIViewController new];
    }

    dispatch_init();

    const auto sampleCount = g_app.config.sampleCount;
    const auto swapInterval = g_app.config.swapInterval;

    _view = [MetalView new];
    _view.preferredFramesPerSecond = 60 / swapInterval;
    _view.device = MTLCreateSystemDefaultDevice();
    _view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    _view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    _view.sampleCount = (NSUInteger) sampleCount;
    _view.userInteractionEnabled = YES;
    _view.multipleTouchEnabled = YES;
    _window.rootViewController.modalPresentationStyle = UIModalPresentationFullScreen;
    _window.rootViewController.view = _view;

    [_window makeKeyAndVisible];
    [_window layoutIfNeeded];

    dispatch_device_ready();

    // Override point for customization after application launch.

    return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application {
// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
// Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    dispatch_event(Event::App(Event::Pause));
}


- (void)applicationDidEnterBackground:(UIApplication*)application {
// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.

}


- (void)applicationWillEnterForeground:(UIApplication*)application {
// Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication*)application {
// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    dispatch_event(Event::App(Event::Resume));
}

- (void)applicationWillTerminate:(UIApplication*)application {
    dispatch_event(Event::App(Event::Close));
// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.

    if (_window != nil) {
        OBJC_RELEASE(_window.rootViewController);
    }
    if (_view != nil) {
        OBJC_RELEASE(_view.device);
    }
    OBJC_RELEASE(_view);
    OBJC_RELEASE(_window);
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application {
/*
 Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
 */
}


@end

void handleTouches(ek::app::Event::Type type, UIView* view, NSSet* touches, UIEvent*) {
    const auto scaleFactor = view.contentScaleFactor;
    for (UITouch* touch in [touches allObjects]) {
        const CGPoint location = [touch locationInView:view];
        ek::app::TouchEvent touchEvent;
        touchEvent.id = (uint64_t) touch;
        touchEvent.x = static_cast<float>(location.x * scaleFactor);
        touchEvent.y = static_cast<float>(location.y * scaleFactor);
        dispatch_event(ek::app::Event::Touch(type, touchEvent));
    }
}

@implementation MetalView

- (void)handleResize {
    // currently all iOS application in fullscreen mode
    auto& app = ek::app::g_app;
    app.fullscreen = true;
    const float scaleFactor = static_cast<float>(self.contentScaleFactor);
    const float drawableWidth = static_cast<float>(self.drawableSize.width);
    const float drawableHeight = static_cast<float>(self.drawableSize.height);
    if (drawableWidth != app.drawableWidth ||
        drawableHeight != app.drawableHeight ||
        scaleFactor != app.dpiScale) {
        app.dpiScale = scaleFactor;
        app.windowWidth = drawableWidth / scaleFactor;
        app.windowHeight = drawableHeight / scaleFactor;
        app.drawableWidth = drawableWidth;
        app.drawableHeight = drawableHeight;
        app.dirtySize = true;
    }
}

- (BOOL)isOpaque {
    return YES;
}

- (void)drawRect:(CGRect)rect {
    (void) rect;

    [self handleResize];
    if(self.currentDrawable != nil && self.currentRenderPassDescriptor != nil) {
        // we need to keep ref for default render pass on current frame
        // it should be checked in RELEASE build to ensure all references are valid
        self.defaultPass = self.currentRenderPassDescriptor;
        dispatch_draw_frame();
    }
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesBegan:touches withEvent:event];
    handleTouches(ek::app::Event::TouchBegin, self, touches, event);
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesMoved:touches withEvent:event];
    handleTouches(ek::app::Event::TouchMove, self, touches, event);
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesEnded:touches withEvent:event];
    handleTouches(ek::app::Event::TouchEnd, self, touches, event);
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesCancelled:touches withEvent:event];
    handleTouches(ek::app::Event::TouchEnd, self, touches, event);
}

@end

