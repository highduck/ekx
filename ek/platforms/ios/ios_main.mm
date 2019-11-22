#import "ios_app_delegate.h"

#include <platform/boot.hpp>

namespace ek {

void start_application() {
    const auto& args = get_program_arguments();
    @autoreleasepool {
        UIApplicationMain(
                args.argc,
                args.argv,
                nil,
                NSStringFromClass([IOSAppDelegate class])
        );
    }
}

}
