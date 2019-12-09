#import "ios_eagl_view.h"

@interface IOSAppDelegate : UIResponder <UIApplicationDelegate>

@property(strong, nonatomic) UIWindow* window;
@property(strong, nonatomic) EAGLView* view;

@end

extern IOSAppDelegate* g_app_delegate;

