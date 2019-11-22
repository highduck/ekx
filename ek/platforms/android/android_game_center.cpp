#include <ek/android.hpp>

namespace ek {

void game_services_init() {
    auto* env = android::get_jni_env();
    auto class_ref = env->FindClass("ekapp/GameServices");

    auto method = env->GetStaticMethodID(class_ref, "init", "()V");
    env->CallStaticVoidMethod(class_ref, method);

    env->DeleteLocalRef(class_ref);
}

void leader_board_show(const char* leader_board_id) {
    auto* env = android::get_jni_env();

    auto class_ref = env->FindClass("ekapp/GameServices");
    auto id_ref = env->NewStringUTF(leader_board_id);

    auto method = env->GetStaticMethodID(class_ref, "leader_board_show",
                                         "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(class_ref, method, id_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(id_ref);
}

void leader_board_submit(const char* leader_board_id, int score) {
    auto* env = android::get_jni_env();

    auto class_ref = env->FindClass("ekapp/GameServices");
    auto id_ref = env->NewStringUTF(leader_board_id);

    auto method = env->GetStaticMethodID(class_ref, "leader_board_submit",
                                         "(Ljava/lang/String;I)V");
    env->CallStaticVoidMethod(class_ref, method, id_ref, score);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(id_ref);
}

void achievement_update(const char* achievement_id, int increment) {
    auto* env = android::get_jni_env();

    auto class_ref = env->FindClass("ekapp/GameServices");
    auto id_ref = env->NewStringUTF(achievement_id);

    auto method = env->GetStaticMethodID(class_ref, "achievement_update",
                                         "(Ljava/lang/String;I)V");
    env->CallStaticVoidMethod(class_ref, method, id_ref, increment);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(id_ref);
}

void achievement_show() {
    auto* env = android::get_jni_env();
    auto class_ref = env->FindClass("ekapp/GameServices");

    auto method = env->GetStaticMethodID(class_ref, "achievement_show", "()V");
    env->CallStaticVoidMethod(class_ref, method);

    env->DeleteLocalRef(class_ref);
}

}