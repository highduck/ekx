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

}

#define EK_JNI(x) extern "C" JNIEXPORT void JNICALL x
#define EK_JNI_INT(x) extern "C" JNIEXPORT jint JNICALL x

#endif