#include "Firebase.h"

#include <ek/app_native.h>

#if defined(__APPLE__)

#if TARGET_OS_IOS || TARGET_OS_TV
#import <Firebase/Firebase.h>
#endif

#elif defined(__EMSCRIPTEN__)

#include <emscripten.h>

//extern "C" void firebase_init(void);

#endif

namespace ek::analytics {

#if defined(__ANDROID__)

void init() {
    auto* env = ek_android_jni();

    auto class_ref = env->FindClass("ek/FirebasePlugin");
    auto method = env->GetStaticMethodID(class_ref, "init", "()V");
    env->CallStaticVoidMethod(class_ref, method);

    env->DeleteLocalRef(class_ref);
}

void screen(const char* name) {
    auto* env = ek_android_jni();
    auto class_ref = env->FindClass("ek/FirebasePlugin");
    auto name_ref = env->NewStringUTF(name);
    auto method = env->GetStaticMethodID(class_ref, "set_screen", "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, name_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(name_ref);
}

void event(const char* action, const char* target) {
    auto* env = ek_android_jni();
    auto class_ref = env->FindClass("ek/FirebasePlugin");
    auto action_ref = env->NewStringUTF(action);
    auto target_ref = env->NewStringUTF(target);

    auto method = env->GetStaticMethodID(class_ref, "send_event",
                                         "(Ljava/lang/String;Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, action_ref, target_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(action_ref);
    env->DeleteLocalRef(target_ref);
}

#elif TARGET_OS_IOS || TARGET_OS_TV

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

#elif defined(__EMSCRIPTEN__)

void init() {
    //firebase_init();
}

void screen(const char* name) {
    (void) name;
}

void event(const char* action, const char* target) {
    (void) action;
    (void) target;
}

#else

void init() {
}

void screen(const char* name) {
    (void) name;
}

void event(const char* action, const char* target) {
    (void) action;
    (void) target;
}

#endif

}