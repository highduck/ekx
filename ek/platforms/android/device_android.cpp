#include <ek/android.hpp>

#include <jni.h>
#include <pthread.h>
#include <string>
#include <array>
#include <vector>

namespace ek {

[[maybe_unused]] std::string get_device_lang() {
    auto* env = android::get_jni_env();
    auto class_ref = env->FindClass("ek/EkDevice");
    auto method = env->GetStaticMethodID(class_ref, "getLanguage", "()Ljava/lang/String;");
    auto rv = (jstring) env->CallStaticObjectMethod(class_ref, method);
    const char* temp_str = env->GetStringUTFChars(rv, nullptr);
    std::string result{temp_str};
    env->ReleaseStringUTFChars(rv, temp_str);
    return result;
}

std::array<int, 4> get_screen_insets() {
    std::array<int, 4> result{0, 0, 0, 0};

    auto* env = android::get_jni_env();
    auto class_ref = env->FindClass("ek/EkDevice");
    auto method = env->GetStaticMethodID(class_ref, "getScreenInsets", "()[I");
    auto rv = (jintArray) env->CallStaticObjectMethod(class_ref, method);
    jsize len = env->GetArrayLength(rv);
    jint* temp_arr = env->GetIntArrayElements(rv, nullptr);
    if (len >= 4) {
        result[0] = temp_arr[0];
        result[1] = temp_arr[1];
        result[2] = temp_arr[2];
        result[3] = temp_arr[3];
    }
    env->ReleaseIntArrayElements(rv, temp_arr, 0);

    return result;
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