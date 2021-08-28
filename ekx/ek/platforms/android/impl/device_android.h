#pragma once

#include <ek/android.hpp>

#include <jni.h>
#include <pthread.h>
#include <string>
#include <array>
#include <vector>
#include <ek/math/vec.hpp>
#include <ek/assert.hpp>

namespace ek {

std::string get_device_lang() {
    auto* env = android::get_jni_env();
    auto class_ref = env->FindClass("ek/EkDevice");
    auto method = env->GetStaticMethodID(class_ref, "getLanguage", "()Ljava/lang/String;");
    auto rv = (jstring) env->CallStaticObjectMethod(class_ref, method);
    const char* temp_str = env->GetStringUTFChars(rv, nullptr);
    std::string result{temp_str};
    env->ReleaseStringUTFChars(rv, temp_str);
    return result;
}

void getScreenInsets(float padding[4]) {
    auto* env = android::get_jni_env();
    auto class_ref = env->FindClass("ek/EkDevice");
    auto method = env->GetStaticMethodID(class_ref, "getScreenInsets", "()[I");
    auto rv = (jintArray) env->CallStaticObjectMethod(class_ref, method);

    const jsize len = env->GetArrayLength(rv);
    if(len >= 4) {
        jint* temp_arr = env->GetIntArrayElements(rv, nullptr);
        EK_ASSERT(temp_arr);

        padding[0] = static_cast<float>(temp_arr[0]);
        padding[1] = static_cast<float>(temp_arr[1]);
        padding[2] = static_cast<float>(temp_arr[2]);
        padding[3] = static_cast<float>(temp_arr[3]);

        env->ReleaseIntArrayElements(rv, temp_arr, 0);
    }
}

void vibrate(int duration_millis) {
    if (duration_millis <= 0) {
        return;
    }
    auto* env = ek::android::get_jni_env();
    auto clazz = env->FindClass("ek/EkDevice");
    auto method = env->GetStaticMethodID(clazz, "vibrate", "(J)V");
    env->CallStaticVoidMethod(clazz, method, (jlong) duration_millis);
}

std::string getDeviceFontPath(const char* fontName) {
    // TODO:
//    return "/system/fonts/Roboto-Regular.ttf";
    return "/system/fonts/DroidSansFallback.ttf";
}

}