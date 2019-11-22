#import "ios_app_delegate.h"
#import "EAGLView.h"

#include <platform/application.hpp>

IOSAppDelegate* g_app_delegate;

using namespace ek;

@implementation IOSAppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    g_app_delegate = self;
    g_app.init();
    
    [self.window makeKeyAndVisible];

    [(EAGLView*) self.window.rootViewController.view startAnimation];
    
    [self.window layoutIfNeeded];

    // Override point for customization after application launch.
    g_app.start();

    return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.

    [(EAGLView*) self.window.rootViewController.view stopAnimation];

    g_app.dispatch({event_type::app_pause});
}


- (void)applicationDidEnterBackground:(UIApplication*)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.

    [(EAGLView*) self.window.rootViewController.view stopAnimation];
}


- (void)applicationWillEnterForeground:(UIApplication*)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.

    [(EAGLView*) self.window.rootViewController.view startAnimation];

    g_app.dispatch({event_type::app_resume});
}


- (void)applicationDidBecomeActive:(UIApplication*)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.

    [(EAGLView*) self.window.rootViewController.view startAnimation];
}

- (void)applicationWillTerminate:(UIApplication*)application {
    [(EAGLView*) self.window.rootViewController.view stopAnimation];

    g_app.dispatch({event_type::app_close});
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application {
    /*
     Free up as much memory as possible by purging cached data objects that can be recreated (or reloaded from disk) later.
     */
}


@end
