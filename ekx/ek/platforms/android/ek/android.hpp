#pragma once

#if defined(__ANDROID__)

#include <android/asset_manager.h>
#include <jni.h>

namespace ek::android {

JNIEnv* get_jni_env();

jobject get_activity();

jobject get_context();

void set_asset_manager(jobject asset_manager);

AAssetManager* get_asset_manager();

jobject assetManagerRef();

}

#define EK_ALOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ek", __VA_ARGS__))
#define EK_ALOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ek", __VA_ARGS__))
#define EK_ALOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "ek", __VA_ARGS__))

#endif