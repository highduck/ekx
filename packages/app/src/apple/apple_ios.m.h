#include <ek/app_native.h>
#include "../intern.h"

void handleTouches(ek_app_event_type type, UIView* view, NSSet* touches, UIEvent* ev_) {
    const CGFloat scaleFactor = view.contentScaleFactor;
    for (UITouch* touch in [touches allObjects]) {
        const CGPoint location = [touch locationInView:view];
        ek_app__process_event((ek_app_event) {.touch = {
                .type = type,
                .id = (uint64_t) touch,
                .x = (float) (location.x * scaleFactor),
                .y = (float) (location.y * scaleFactor)
        }});
    }
}

@implementation AppDelegate

@synthesize window;
@synthesize view;

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    ek_app_delegate = self;

// maybe if you have got this => disable new iOS 13.0 Scene Manifest behavior in Info.plist
//EKAPP_ASSERT(self.window != nil);

    if (!self.window) {
        self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
        self.window.rootViewController = [UIViewController new];
    }

    ek_app__init();
    
    // currently all iOS application in fullscreen mode
    ek_app.state |= EK_APP_STATE_FULLSCREEN;
    
    ek_app_main();
    if (ek_app.state & EK_APP_STATE_EXIT_PENDING) {
        return NO;
    }

    const int sampleCount = ek_app.config.sample_count;
    const int swapInterval = ek_app.config.swap_interval;

    self.view = [MetalView new];
    self.view.preferredFramesPerSecond = 60 / swapInterval;
    self.view.device = MTLCreateSystemDefaultDevice();
    self.view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
    self.view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    self.view.sampleCount = (NSUInteger) sampleCount;
    self.view.userInteractionEnabled = YES;
    self.view.multipleTouchEnabled = YES;
    self.window.rootViewController.modalPresentationStyle = UIModalPresentationFullScreen;
    self.window.rootViewController.view = self.view;

    [self.window makeKeyAndVisible];
    [self.window layoutIfNeeded];

    ek_app__notify_ready();

    // Override point for customization after application launch.

    return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application {
// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
// Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    ek_app__process_event((ek_app_event){.type = EK_APP_EVENT_PAUSE});
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
    ek_app__process_event((ek_app_event){.type = EK_APP_EVENT_RESUME});
}

- (void)applicationWillTerminate:(UIApplication*)application {
    ek_app__process_event((ek_app_event){.type = EK_APP_EVENT_CLOSE});
// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.

    if (self.window != nil) {
        EK_OBJC_RELEASE(self.window.rootViewController);
    }
    if (self.view != nil) {
        EK_OBJC_RELEASE(self.view.device);
    }
    EK_OBJC_RELEASE(self.view);
    EK_OBJC_RELEASE(self.window);
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application {
/*
 Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
 */
}


@end

@implementation MetalView

@synthesize defaultPass;

- (void)handleResize {
    const CGFloat scale = self.contentScaleFactor;
    const CGFloat width = self.drawableSize.width;
    const CGFloat height = self.drawableSize.height;
    ek_app_viewport viewport = (ek_app_viewport){
            .width = (float)width,
            .height = (float)height,
            .scale = (float)scale,
            .insets = {0.0f, 0.0f, 0.0f, 0.0f}
    };
    if (@available(iOS 11.0, *)) {
        UIEdgeInsets safeAreaInsets = self.safeAreaInsets;
        viewport.insets[0] = (float)(safeAreaInsets.left * scale);
        viewport.insets[1] = (float)(safeAreaInsets.top * scale);
        viewport.insets[2] = (float)(safeAreaInsets.right * scale);
        viewport.insets[3] = (float)(safeAreaInsets.bottom * scale);
    }
    ek_app__update_viewport(viewport);
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
        ek_app__process_frame();
    }
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesBegan:touches withEvent:event];
    handleTouches(EK_APP_EVENT_TOUCH_START, self, touches, event);
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesMoved:touches withEvent:event];
    handleTouches(EK_APP_EVENT_TOUCH_MOVE, self, touches, event);
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesEnded:touches withEvent:event];
    handleTouches(EK_APP_EVENT_TOUCH_END, self, touches, event);
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
    [super touchesCancelled:touches withEvent:event];
    handleTouches(EK_APP_EVENT_TOUCH_END, self, touches, event);
}

@end

