#pragma once

#include <ek/app/prefs.hpp>
#include <ek/app/Platform.h>

namespace ek {

void set_user_string(const char* key, const char* str) {
    auto* env = app::getJNIEnv();

    auto class_ref = env->FindClass("ek/Preferences");
    auto key_ref = env->NewStringUTF(key);
    auto val_ref = env->NewStringUTF(str);

    auto method = env->GetStaticMethodID(class_ref, "set_string", "(Ljava/lang/String;Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, key_ref, val_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(key_ref);
    env->DeleteLocalRef(val_ref);
}

std::string get_user_string(const char* key, const char* default_value) {
    auto* env = app::getJNIEnv();

    auto class_ref = env->FindClass("ek/Preferences");
    auto key_ref = env->NewStringUTF(key);
    // TODO: set default only when value is not presented (return null from jni)
    auto default_value_ref = env->NewStringUTF(default_value);

    auto method = env->GetStaticMethodID(class_ref, "get_string", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
    auto result = (jstring)env->CallStaticObjectMethod(class_ref, method, key_ref, default_value_ref);

    jboolean is_copy;
    const char* data = env->GetStringUTFChars(result, &is_copy);
    const jsize size = env->GetStringUTFLength(result);
    const std::string result_str{data, size_t(size)};
    env->ReleaseStringUTFChars(result, data);
    env->DeleteLocalRef(result);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(key_ref);
    env->DeleteLocalRef(default_value_ref);

    return result_str;
}

void set_user_preference(const char* key, int value) {
    auto* env = app::getJNIEnv();

    auto class_ref = env->FindClass("ek/Preferences");
    auto key_ref = env->NewStringUTF(key);

    auto method = env->GetStaticMethodID(class_ref, "set_int", "(Ljava/lang/String;I)V");
    env->CallStaticVoidMethod(class_ref, method, key_ref, value);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(key_ref);
}

int get_user_preference(const char* key, int default_value) {
    auto* env = app::getJNIEnv();

    auto class_ref = env->FindClass("ek/Preferences");
    auto key_ref = env->NewStringUTF(key);

    auto method = env->GetStaticMethodID(class_ref, "get_int", "(Ljava/lang/String;I)I");
    auto result = env->CallStaticIntMethod(class_ref, method, key_ref, default_value);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(key_ref);
    return result;
}

}