#pragma once

#if defined(__ANDROID__)

#define EKAPP_ANDROID  1

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>

#define EKAPP_LOG_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ekapp", __VA_ARGS__))
#define EKAPP_LOG_WARN(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ekapp", __VA_ARGS__))
#define EKAPP_LOG_ERROR(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "ekapp", __VA_ARGS__))

#elif defined(__APPLE__)

#define EKAPP_APPLE  1

#define EKAPP_LOG_INFO(...) (NSLog(@"[ekapp] I: "@__VA_ARGS__))
#define EKAPP_LOG_WARN(...) (NSLog(@"[ekapp] W: "@__VA_ARGS__))
#define EKAPP_LOG_ERROR(...) (NSLog(@"[ekapp] E: "@__VA_ARGS__))

#include <TargetConditionals.h>

#if TARGET_OS_IOS || TARGET_OS_TV
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

#import <MetalKit/MetalKit.h>

#if __has_feature(objc_arc)
#define EKAPP_RELEASE(obj) { obj = nil; }
#else
#define EKAPP_RELEASE(obj) { [obj release]; obj = nil; }
#endif

#endif

namespace ek::app {

#if defined(__ANDROID__)

JNIEnv* getJNIEnv();

jobject get_activity();

jobject get_context();

void set_asset_manager(jobject asset_manager);

AAssetManager* get_asset_manager();

jobject assetManagerRef();

#endif

#if defined(__APPLE__)

void* getMetalDevice();

const void* getMetalRenderPass();

const void* getMetalDrawable();

#endif

}

#if defined(__APPLE__)

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

extern AppDelegate* gAppDelegate;

#endif
