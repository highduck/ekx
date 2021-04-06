#import <MetalKit/MetalKit.h>

@interface IOSView : MTKView {}
@end

@interface IOSAppDelegate : UIResponder <UIApplicationDelegate>

@property(strong, nonatomic) UIWindow* window;
@property(strong, nonatomic) IOSView* view;

@end

extern IOSAppDelegate* g_app_delegate;
extern IOSAppDelegate* gAppDelegate;

