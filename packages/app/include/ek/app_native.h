#ifndef EK_APP_NATIVE_H
#define EK_APP_NATIVE_H

/**
 * App's target-specific API.
 * Provide interface for making app's extensions.
 */

#include <ek/app.h>

//region Android

#ifdef __ANDROID__

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

JNIEnv* ek_android_jni(void);

jobject ek_android_activity(void);

jobject ek_android_context(void);

AAssetManager* ek_android_assets(void);

jobject ek_android_assets_object(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ANDROID__
//endregion

//region iOS/macOS
#ifdef __APPLE__

#include <TargetConditionals.h>

#if TARGET_OS_IOS || TARGET_OS_TV
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

#import <MetalKit/MetalKit.h>

#if __has_feature(objc_arc)
#define EK_OBJC_RELEASE(obj) { obj = nil; }
#else
#define EK_OBJC_RELEASE(obj) { [obj release]; obj = nil; }
#endif

// we use common metal view for iOS and macOS
@interface MetalView : MTKView {}
@property(strong, nonatomic) MTLRenderPassDescriptor* defaultPass;
@end

#if TARGET_OS_IOS || TARGET_OS_TV

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property(strong, nonatomic) UIWindow* window;
@property(strong, nonatomic) MetalView* view;

@end

#else

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>

@property(strong, nonatomic) NSApplication* application;
@property(strong, nonatomic) NSWindow* window;
@property(strong, nonatomic) MetalView* view;

@end

#endif

extern AppDelegate* ek_app_delegate;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void* ek_metal__device(void);

const void* ek_metal__render_pass(void);

const void* ek_metal__drawable(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __APPLE__
//endregion APPLE

#endif // EK_APP_NATIVE_H
