#include <platform/analytics.hpp>
#include <ek/config/detect_platform.hpp>

#if EK_ANDROID

#include <platform/ek_android.h>

namespace ek::analytics {

void init() {
    auto* env = android::get_jni_env();

    auto class_ref = env->FindClass("ekapp/Analytics");
    auto method = env->GetStaticMethodID(class_ref, "init", "()V");
    env->CallStaticVoidMethod(class_ref, method);

    env->DeleteLocalRef(class_ref);
}

void screen(const char* name) {
    auto* env = android::get_jni_env();
    auto class_ref = env->FindClass("ekapp/Analytics");
    auto name_ref = env->NewStringUTF(name);
    auto method = env->GetStaticMethodID(class_ref, "set_screen", "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, name_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(name_ref);
}

void event(const char* action, const char* target) {
    auto* env = android::get_jni_env();
    auto class_ref = env->FindClass("ekapp/Analytics");
    auto action_ref = env->NewStringUTF(action);
    auto target_ref = env->NewStringUTF(target);

    auto method = env->GetStaticMethodID(class_ref, "send_event",
                                         "(Ljava/lang/String;Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, action_ref, target_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(action_ref);
    env->DeleteLocalRef(target_ref);
}

}

#elif EK_IOS

///////// see `platforms/analytics.mm`

#else

namespace ek::analytics {

void init() {}

void screen(const char*) {}

void event(const char*, const char*) {}

}

#endif