#pragma once

#import <UIKit/UIKit.h>
#import "../app_apple.h"

#include <ek/app/app.hpp>

using namespace ek::app;

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

    _view = [MetalView new];
    _view.preferredFramesPerSecond = 60 / 1;/*swap_interval*/
    _view.device = MTLCreateSystemDefaultDevice();
    _view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    _view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    _view.sampleCount = (NSUInteger) 1; /*sample_count*/
//    _view.autoResizeDrawable = false;
    _view.userInteractionEnabled = YES;
    _view.multipleTouchEnabled = YES;
    _window.rootViewController.modalPresentationStyle = UIModalPresentationFullScreen;
    _window.rootViewController.view = _view;

    g_app.view_context_ = (__bridge void*) _view;

    [_window makeKeyAndVisible];
    [_window layoutIfNeeded];

// Override point for customization after application launch.
    dispatch_device_ready();

    return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application {
// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
// Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    dispatch_event({event_type::app_pause});
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
    dispatch_event({event_type::app_resume});
}

- (void)applicationWillTerminate:(UIApplication*)application {
    dispatch_event({event_type::app_close});
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

void handle_touches(event_type type, UIView* view, NSSet* touches, UIEvent* event) {
    event_t ev{type};
    const auto scale_factor = view.contentScaleFactor;
    for (UITouch* touch in [touches allObjects]) {
        const CGPoint location = [touch locationInView:view];
        ev.id = (uint64_t) touch;
        ev.pos.x = static_cast<float>(location.x * scale_factor);
        ev.pos.y = static_cast<float>(location.y * scale_factor);
        dispatch_event(ev);
    }
}

@implementation MetalView

- (void)handleResize {
//CGRect screen_rect = UIScreen.mainScreen.bounds;
    float drawableWidth = static_cast<float>(self.drawableSize.width);
    float drawableHeight = static_cast<float>(self.drawableSize.height);
    if (drawableWidth != g_app.drawable_size.x || drawableHeight != g_app.drawable_size.y) {
        g_app.window_size.x = drawableWidth;
        g_app.window_size.x = drawableHeight;
        g_app.drawable_size.x = drawableWidth;
        g_app.drawable_size.y = drawableHeight;
        g_app.content_scale = 1.0f;
        g_app.size_changed = true;
    }
}

- (BOOL)isOpaque {
    return YES;
}

- (void)drawRect:(CGRect)rect {
    (void) rect;

    [self handleResize];
    dispatch_draw_frame();
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesBegan:touches withEvent:event];
    handle_touches(event_type::touch_begin, self, touches, event);
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesMoved:touches withEvent:event];
    handle_touches(event_type::touch_move, self, touches, event);
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesEnded:touches withEvent:event];
    handle_touches(event_type::touch_end, self, touches, event);
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesCancelled:touches withEvent:event];
    handle_touches(event_type::touch_end, self, touches, event);
}

@end

