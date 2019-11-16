#import "AppDelegate.h"

#include <platform/boot.h>

namespace ek {

void start_application() {
    const auto& args = get_program_c_arguments();
    @autoreleasepool {
        UIApplicationMain(
                static_cast<int>(args.size()),
                const_cast<char**>(args.data()),
                nil,
                NSStringFromClass([AppDelegate class])
        );
    }
}

}
