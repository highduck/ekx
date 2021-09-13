#pragma once

#include <ek/app/Platform.h>

namespace ek::app {

int openURL(const char* url) {
    auto* env = app::getJNIEnv();
    auto jURL = env->NewStringUTF(url);
    auto cls = env->FindClass("ek/EkDevice");

    auto method = env->GetStaticMethodID(cls, "openURL", "(Ljava/lang/String;)I");
    int result = env->CallStaticIntMethod(cls, method, jURL);

    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(jURL);

    return result;
}

const char* getPreferredLang() {
    auto* env = getJNIEnv();
    auto cls = env->FindClass("ek/EkDevice");
    auto method = env->GetStaticMethodID(cls, "getLanguage", "()Ljava/lang/String;");
    auto rv = (jstring) env->CallStaticObjectMethod(cls, method);
    const char* temp = env->GetStringUTFChars(rv, nullptr);
    static char lang[8];
    lang[0] = '\0';
    strncat(lang, temp, 7);
    env->ReleaseStringUTFChars(rv, temp);
    return lang;
}

const float* getScreenInsets() {
    auto* env = getJNIEnv();
    auto class_ref = env->FindClass("ek/EkDevice");
    auto method = env->GetStaticMethodID(class_ref, "getScreenInsets", "()[I");
    auto rv = (jintArray) env->CallStaticObjectMethod(class_ref, method);

    const jsize len = env->GetArrayLength(rv);
    if (len >= 4) {
        jint* arr = env->GetIntArrayElements(rv, nullptr);
        EKAPP_ASSERT(arr);

        static float insets[4];
        insets[0] = static_cast<float>(arr[0]);
        insets[1] = static_cast<float>(arr[1]);
        insets[2] = static_cast<float>(arr[2]);
        insets[3] = static_cast<float>(arr[3]);

        env->ReleaseIntArrayElements(rv, arr, 0);

        return insets;
    }
    return nullptr;
}

int vibrate(int millis) {
    auto* env = getJNIEnv();
    auto cls = env->FindClass("ek/EkDevice");
    auto method = env->GetStaticMethodID(cls, "vibrate", "(J)I");
    auto result = env->CallStaticIntMethod(cls, method, (jlong) millis);
    env->DeleteLocalRef(cls);
    return result;
}

const char* getSystemFontPath(const char* fontName) {
    (void) fontName;
    // TODO:
    return "/system/fonts/DroidSansFallback.ttf";
    // return "/system/fonts/Roboto-Regular.ttf";
}

}