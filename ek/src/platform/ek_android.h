#pragma once

namespace ek {}

#if defined(__ANDROID__)

#include <android/asset_manager.h>
#include <jni.h>

namespace ek {
    namespace android {
        JNIEnv* get_jni_env();
        jobject get_activity();
        jobject get_context();
        void set_asset_manager(jobject asset_manager);
        AAssetManager* get_asset_manager();
    }
}

extern "C" JNIEXPORT void JNICALL Java_ekapp_EKPlatform_on_1startup(JNIEnv *env, jclass type);
extern "C" JNIEXPORT void JNICALL Java_ekapp_EKPlatform_on_1ready(JNIEnv *env, jclass type);
extern "C" JNIEXPORT void JNICALL Java_ekapp_EKPlatform_handle_1back_1button(JNIEnv *env, jclass type);
extern "C" JNIEXPORT void JNICALL Java_ekapp_EKPlatform_set_1assets_1manager(JNIEnv *env, jclass type, jobject assets);
extern "C" JNIEXPORT void JNICALL Java_ekapp_EKPlatform_handle_1resize(JNIEnv *env, jclass type, jint width, jint height,
                                     jfloat scaleFactor);
extern "C" JNIEXPORT void JNICALL Java_ekapp_EKPlatform_handle_1touch_1event(JNIEnv *env, jclass type_, jint type, jint id, jfloat x, jfloat y);
extern "C" JNIEXPORT void JNICALL Java_ekapp_EKPlatform_handle_1pause(JNIEnv *env, jclass type);
extern "C" JNIEXPORT void JNICALL Java_ekapp_EKPlatform_handle_1resume(JNIEnv *env, jclass type);
extern "C" JNIEXPORT void JNICALL Java_ekapp_EKPlatform_handle_1enter_1frame(JNIEnv *env, jclass type);

#endif