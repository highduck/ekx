#import "ios_app_delegate.h"

#include <ek/app/app.hpp>

IOSAppDelegate* g_app_delegate;

using namespace ek::app;

@implementation IOSAppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    g_app_delegate = self;

    // maybe if you have got this => disable new iOS 13.0 Scene Manifest behavior in Info.plist
    //assert(self.window != nil);

    if (!self.window) {
        self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
        self.window.rootViewController = [UIViewController new];
    }

    dispatch_init();

    _view = [[EAGLView alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.rootViewController.view = _view;

    g_app.view_context_ = (__bridge void*) _view;

    [_window makeKeyAndVisible];
    [_view startAnimation];
    [_window layoutIfNeeded];

    // Override point for customization after application launch.
    dispatch_device_ready();

    return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.

    [_view stopAnimation];

    dispatch_event({event_type::app_pause});
}


- (void)applicationDidEnterBackground:(UIApplication*)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.

    [_view stopAnimation];
}


- (void)applicationWillEnterForeground:(UIApplication*)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.

    [_view startAnimation];

    dispatch_event({event_type::app_resume});
}


- (void)applicationDidBecomeActive:(UIApplication*)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.

    [_view startAnimation];
}

- (void)applicationWillTerminate:(UIApplication*)application {
    [_view stopAnimation];

    dispatch_event({event_type::app_close});
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


@end
