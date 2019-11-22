#import <UIKit/UIKit.h>

@interface IOSAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

@end

extern IOSAppDelegate* g_app_delegate;

