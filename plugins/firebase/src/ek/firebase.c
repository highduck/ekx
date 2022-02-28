#include "firebase.h"

#if defined(__APPLE__)

#include <TargetConditionals.h>

#if TARGET_OS_IOS || TARGET_OS_TV

#include <ek/app_native.h>

#import <Firebase/Firebase.h>

#endif // TARGET_OS_IOS || TARGET_OS_TV

#elif defined(__ANDROID__)

#include <ek/app_native.h>

#elif defined(__EMSCRIPTEN__)

#include <emscripten.h>

#endif

#if defined(__ANDROID__)

bool firebase(firebase_cmd_t cmd) {
    bool result = false;
    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, "ek/FirebasePlugin");
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "call", "(I)V");
    (*env)->CallStaticVoidMethod(env, class_ref, method, cmd);
    // TODO: result from Java
    result = true;
    (*env)->DeleteLocalRef(env, class_ref);
    return result;
}

void firebase_screen(const char* name) {
    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, "ek/FirebasePlugin");
    jstring name_ref = (*env)->NewStringUTF(env, name);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "set_screen", "(Ljava/lang/String;)V");
    (*env)->CallStaticVoidMethod(env, class_ref, method, name_ref);
    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, name_ref);
}

void firebase_event(const char* action, const char* target) {
    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, "ek/FirebasePlugin");
    jstring action_ref = (*env)->NewStringUTF(env, action);
    jstring target_ref = (*env)->NewStringUTF(env, target);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "send_event",
                                                 "(Ljava/lang/String;Ljava/lang/String;)V");
    (*env)->CallStaticVoidMethod(env, class_ref, method, action_ref, target_ref);
    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, action_ref);
    (*env)->DeleteLocalRef(env, target_ref);
}

#elif (TARGET_OS_IOS || TARGET_OS_TV)

bool firebase(firebase_cmd_t cmd) {
    if(cmd == FIREBASE_CMD_INIT) {
        [FIRApp configure];
        return true;
    }
    return false;
}

void firebase_screen(const char* name) {
    NSString* nsScreenName = name != nil ? [NSString stringWithUTF8String: name] : @"unknown";
    [FIRAnalytics logEventWithName:kFIREventScreenView
    parameters:@{
        kFIRParameterScreenName : nsScreenName
    }];
}

void firebase_event(const char* action, const char* target) {
    NSString* nsAction = action != nil ? [NSString stringWithUTF8String: action] : @"unknown";
    NSString* nsTarget = target != nil ? [NSString stringWithUTF8String: target] : @"unknown";

    [FIRAnalytics logEventWithName: nsAction
    parameters: @{
        kFIRParameterItemName: nsTarget,
    }];
}

#elif defined(__EMSCRIPTEN__)

extern bool firebase_js(uint32_t cmd);

bool firebase(firebase_cmd_t cmd) {
    return firebase_js(cmd);
}

void firebase_screen(const char* name) {
    (void) name;
}

void firebase_event(const char* action, const char* target) {
    (void) action;
    (void) target;
}

#else

bool firebase(firebase_cmd_t cmd) {
    (void)(cmd);
    return false;
}

void firebase_screen(const char* name) {
    (void) name;
}

void firebase_event(const char* action, const char* target) {
    (void) action;
    (void) target;
}

#endif
