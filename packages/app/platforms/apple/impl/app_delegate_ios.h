#pragma once

#include <ek/app/Platform.h>

namespace ek::app {

void handleTouches(EventType type, UIView* view, NSSet* touches, UIEvent*) {
    const auto scaleFactor = view.contentScaleFactor;
    for (UITouch* touch in [touches allObjects]) {
        const CGPoint location = [touch locationInView:view];
        TouchEvent ev{
                type,
                (uint64_t) touch,
                static_cast<float>(location.x * scaleFactor),
                static_cast<float>(location.y * scaleFactor)
        };
        processEvent(ev);
    }
}

}

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    using namespace ek::app;

    gAppDelegate = self;

// maybe if you have got this => disable new iOS 13.0 Scene Manifest behavior in Info.plist
//EKAPP_ASSERT(self.window != nil);

    if (!self.window) {
        self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
        self.window.rootViewController = [UIViewController new];
    }

    ek::app::main();
    if (g_app.exitRequired) {
        return NO;
    }

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

    notifyReady();

    // Override point for customization after application launch.

    return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application {
// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
// Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    using namespace ek::app;
    processEvent(EventType::Pause);
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
    using namespace ek::app;
    processEvent(EventType::Resume);
}

- (void)applicationWillTerminate:(UIApplication*)application {
    using namespace ek::app;
    processEvent(EventType::Close);
// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.

    if (_window != nil) {
        EKAPP_RELEASE(_window.rootViewController);
    }
    if (_view != nil) {
        EKAPP_RELEASE(_view.device);
    }
    EKAPP_RELEASE(_view);
    EKAPP_RELEASE(_window);
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application {
/*
 Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
 */
}


@end

@implementation MetalView

- (void)handleResize {
    using namespace ek::app;
    // currently all iOS application in fullscreen mode
    g_app.fullscreen = true;
    const float scaleFactor = static_cast<float>(self.contentScaleFactor);
    const float drawableWidth = static_cast<float>(self.drawableSize.width);
    const float drawableHeight = static_cast<float>(self.drawableSize.height);
    if (drawableWidth != g_app.drawableWidth ||
        drawableHeight != g_app.drawableHeight ||
        scaleFactor != g_app.dpiScale) {
        g_app.dpiScale = scaleFactor;
        g_app.windowWidth = drawableWidth / scaleFactor;
        g_app.windowHeight = drawableHeight / scaleFactor;
        g_app.drawableWidth = drawableWidth;
        g_app.drawableHeight = drawableHeight;
        g_app.dirtySize = true;
    }
}

- (BOOL)isOpaque {
    return YES;
}

- (void)drawRect:(CGRect)rect {
    (void) rect;
    [self handleResize];
    if (self.currentDrawable != nil && self.currentRenderPassDescriptor != nil) {
        // we need to keep ref for default render pass on current frame
        // it should be checked in RELEASE build to ensure all references are valid
        self.defaultPass = self.currentRenderPassDescriptor;
        ek::app::processFrame();
    }
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
    using namespace ek::app;
    [super touchesBegan:touches withEvent:event];
    handleTouches(EventType::TouchStart, self, touches, event);
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
    using namespace ek::app;
    [super touchesMoved:touches withEvent:event];
    handleTouches(EventType::TouchMove, self, touches, event);
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
    using namespace ek::app;
    [super touchesEnded:touches withEvent:event];
    handleTouches(EventType::TouchEnd, self, touches, event);
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
    using namespace ek::app;
    [super touchesCancelled:touches withEvent:event];
    handleTouches(EventType::TouchEnd, self, touches, event);
}

@end

