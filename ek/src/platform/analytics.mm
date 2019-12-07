#include <platform/analytics.hpp>
#include <ek/util/detect_platform.hpp>

#if EK_IOS

#import <Firebase/Firebase.h>

namespace ek::analytics {

void init() {
    [FIRApp configure];
}

void screen(const char* name) {
    NSString* screen_name = name != nullptr ? [NSString stringWithUTF8String: name] : @"unknown";
    [FIRAnalytics setScreenName: screen_name screenClass: nil];
}

void event(const char* action, const char* target) {
    NSString* action_ns = action != NULL ? [NSString stringWithUTF8String: action] : @"unknown";
    NSString* target_ns = target != NULL ? [NSString stringWithUTF8String: target] : @"unknown";

    [FIRAnalytics logEventWithName: action_ns
                        parameters: @{
                                kFIRParameterItemName: target_ns,
                        }];
}

}

#endif