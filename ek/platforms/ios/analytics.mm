#include <ek/ext/analytics/analytics.hpp>

#import <Firebase/Firebase.h>

namespace ek::analytics {

void init() {
    [FIRApp configure];
}

void screen(const char* name) {
    NSString* nsScreenName = name != nullptr ? [NSString stringWithUTF8String: name] : @"unknown";
    [FIRAnalytics logEventWithName:kFIREventScreenView
                        parameters:@{
                            kFIRParameterScreenName : nsScreenName
                        }];
}

void event(const char* action, const char* target) {
    NSString* nsAction = action != nullptr ? [NSString stringWithUTF8String: action] : @"unknown";
    NSString* nsTarget = target != nullptr ? [NSString stringWithUTF8String: target] : @"unknown";

    [FIRAnalytics logEventWithName: nsAction
                        parameters: @{
                                kFIRParameterItemName: nsTarget,
                        }];
}

}
