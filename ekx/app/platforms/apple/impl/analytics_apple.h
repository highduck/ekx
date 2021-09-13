#pragma once

#if TARGET_OS_IOS || TARGET_OS_TV
#import <Firebase/Firebase.h>
#endif

namespace ek::analytics {

void init() {
#if TARGET_OS_IOS || TARGET_OS_TV
    [FIRApp configure];
#endif
}

void screen(const char* name) {
#if TARGET_OS_IOS || TARGET_OS_TV
    NSString* nsScreenName = name != nullptr ? [NSString stringWithUTF8String: name] : @"unknown";
    [FIRAnalytics logEventWithName:kFIREventScreenView
    parameters:@{
        kFIRParameterScreenName : nsScreenName
    }];
#else
    EK_TRACE("[analytics] screen: %s", name);
#endif
}

void event(const char* action, const char* target) {
#if TARGET_OS_IOS || TARGET_OS_TV
    NSString* nsAction = action != nullptr ? [NSString stringWithUTF8String: action] : @"unknown";
    NSString* nsTarget = target != nullptr ? [NSString stringWithUTF8String: target] : @"unknown";

    [FIRAnalytics logEventWithName: nsAction
    parameters: @{
        kFIRParameterItemName: nsTarget,
    }];
#else
    EK_TRACE("[analytics] action: %s, target: %s", action, target);
#endif
}

}
