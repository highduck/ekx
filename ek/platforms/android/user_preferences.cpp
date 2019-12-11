#include <ek/util/base64.hpp>
#include <ek/android.hpp>
#include <cassert>

namespace ek {

void set_user_data(const std::string& key, const std::vector<uint8_t>& buffer) {
    assert(!key.empty());
    set_user_string(key, base64::encode(buffer));
}

std::vector<uint8_t> get_user_data(const std::string& key) {
    return base64::decode(get_user_string(key));
}

void set_user_preference(const std::string& key, int value) {
    auto* env = android::get_jni_env();

    auto class_ref = env->FindClass("ek/Preferences");
    auto key_ref = env->NewStringUTF(key.c_str());

    auto method = env->GetStaticMethodID(class_ref, "set_int", "(Ljava/lang/String;I)V");
    env->CallStaticVoidMethod(class_ref, method, key_ref, value);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(key_ref);
}

int get_user_preference(const std::string& key, int default_value) {
    auto* env = android::get_jni_env();

    auto class_ref = env->FindClass("ek/Preferences");
    auto key_ref = env->NewStringUTF(key.c_str());

    auto method = env->GetStaticMethodID(class_ref, "get_int", "(Ljava/lang/String;I)I");
    auto result = env->CallStaticIntMethod(class_ref, method, key_ref, default_value);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(key_ref);
    return result;
}

void set_user_string(const std::string& key, const std::string& str) {
    auto* env = android::get_jni_env();

    auto class_ref = env->FindClass("ek/Preferences");
    auto key_ref = env->NewStringUTF(key.c_str());
    auto val_ref = env->NewStringUTF(str.c_str());

    auto method = env->GetStaticMethodID(class_ref, "set_string", "(Ljava/lang/String;Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, key_ref, val_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(key_ref);
    env->DeleteLocalRef(val_ref);
}

std::string get_user_string(const std::string& key, const std::string& default_value) {
    auto* env = android::get_jni_env();

    auto class_ref = env->FindClass("ek/Preferences");
    auto key_ref = env->NewStringUTF(key.c_str());
    auto default_value_ref = env->NewStringUTF(default_value.c_str());

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

}