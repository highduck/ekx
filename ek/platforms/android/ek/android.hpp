#pragma once

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

#endif