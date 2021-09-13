#pragma once

#include <ek/app/Platform.h>

namespace ek {

void sharing_rate_us(const char* app_id) {
    auto* env = app::getJNIEnv();
    auto app_id_ref = env->NewStringUTF(app_id);
    auto class_ref = env->FindClass("ek/Sharing");

    auto method = env->GetStaticMethodID(class_ref, "rate_us", "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, app_id_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(app_id_ref);
}

void sharing_send_message(const char* text) {
    auto* env = app::getJNIEnv();
    auto text_ref = env->NewStringUTF(text);
    auto class_ref = env->FindClass("ek/Sharing");

    auto method = env->GetStaticMethodID(class_ref, "send_message", "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, text_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(text_ref);
}

}