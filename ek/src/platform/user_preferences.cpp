#include "user_preferences.hpp"

#include <ek/util/base64.hpp>
#include <cassert>

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>

#elif defined(__ANDROID__)

#include <ek/android.hpp>

#endif

namespace ek {

void set_user_data(const std::string& key, const std::vector<uint8_t>& buffer) {
    assert(!key.empty());
    set_user_string(key, ek::base64::encode(buffer));
}

std::vector<uint8_t> get_user_data(const std::string& key) {
    return ek::base64::decode(get_user_string(key));
}

#if defined(__EMSCRIPTEN__)

void set_user_preference(const std::string& key, int value) {
    EM_ASM({
        window.localStorage.setItem(UTF8ToString($0), $1);
    }, key.c_str(), value);
}

int get_user_preference(const std::string& key, int default_value) {
    return EM_ASM_INT({
        var item = window.localStorage.getItem(UTF8ToString($0));
        if (item != null) {
            var val = parseInt(item);
            if (val != null) {
                return val;
            }
        }
        return $1;
    }, key.c_str(), default_value);
}

void set_user_string(const std::string& key, const std::string& value) {
    EM_ASM({
        window.localStorage.setItem(UTF8ToString($0), UTF8ToString($1));
    }, key.c_str(), value.c_str());
}

std::string get_user_string(const std::string& key, const std::string& default_value) {
    char* buffer = (char*)EM_ASM_INT({
        var item = window.localStorage.getItem(UTF8ToString($0));
        if (item != null) {
            var lengthBytes = lengthBytesUTF8(item) + 1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            return stringToUTF8(item, stringOnWasmHeap, lengthBytes);
        }
        return 0;
    }, key.c_str());
    if(buffer) {
        std::string str{buffer};
        free(buffer);
        return str;
    }
    return default_value;
}

#elif defined(__ANDROID__)

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

#elif defined(_WIN32) || defined(_WIN64)

void set_user_preference(const std::string& key, int value) {
}

int get_user_preference(const std::string& key, int default_value) {
    return default_value;
}

void set_user_string(const std::string& key, const std::string& str) {
}

std::string get_user_string(const std::string& key, const std::string& default_value) {
    return default_value;
}

#endif

}