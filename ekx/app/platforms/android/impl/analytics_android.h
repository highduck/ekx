#pragma once

#include <ek/app/Platform.h>
#include <ek/debug.hpp>

namespace ek::analytics {

void init() {
    EK_TRACE << "analytics initialize";
    auto* env = app::getJNIEnv();

    auto class_ref = env->FindClass("ek/Analytics");
    auto method = env->GetStaticMethodID(class_ref, "init", "()V");
    env->CallStaticVoidMethod(class_ref, method);

    env->DeleteLocalRef(class_ref);
}

void screen(const char* name) {
    EK_TRACE("[analytics] screen: %s", name);

    auto* env = app::getJNIEnv();
    auto class_ref = env->FindClass("ek/Analytics");
    auto name_ref = env->NewStringUTF(name);
    auto method = env->GetStaticMethodID(class_ref, "set_screen", "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, name_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(name_ref);
}

void event(const char* action, const char* target) {
    EK_TRACE("[analytics] action: %s, target: %s", action, target);

    auto* env = app::getJNIEnv();
    auto class_ref = env->FindClass("ek/Analytics");
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