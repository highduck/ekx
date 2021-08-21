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

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ek", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "ek", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "ek", __VA_ARGS__))

#define EK_JNI_VOID(x, ...) \
    extern "C" JNIEXPORT void JNICALL Java_ek_EkPlatform_##x(JNIEnv *env, jclass cls, __VA_ARGS__)

//#define EK_JNI_INT(x, ...) \
//    extern "C" JNIEXPORT jint JNICALL Java_ek_EkPlatform_##x(JNIEnv *env, jclass cls, __VA_ARGS__)

#endif