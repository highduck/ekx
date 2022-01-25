#include <ek/app_native.h>
#include <ek/assert.h>
#include <ek/log.h>

#define CLASS_PATH "ek/GameServices"

void ek_game_services_init() {
    log_debug("game-services initialize");

    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, CLASS_PATH);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "init", "()V");
    (*env)->CallStaticVoidMethod(env, class_ref, method);
    (*env)->DeleteLocalRef(env, class_ref);
}

void ek_leaderboard_show(const char* id) {
    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, CLASS_PATH);
    jstring id_ref = (*env)->NewStringUTF(env, id);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "leader_board_show", "(Ljava/lang/String;)V");
    (*env)->CallStaticVoidMethod(env, class_ref, method, id_ref);
    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, id_ref);
}

void ek_leaderboard_submit(const char* id, int score) {
    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, CLASS_PATH);
    jstring id_ref = (*env)->NewStringUTF(env, id);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "leader_board_submit", "(Ljava/lang/String;I)V");
    (*env)->CallStaticVoidMethod(env, class_ref, method, id_ref, score);
    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, id_ref);
}

void ek_achievement_update(const char* id, int increment) {
    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, CLASS_PATH);
    jstring id_ref = (*env)->NewStringUTF(env, id);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "achievement_update", "(Ljava/lang/String;I)V");
    (*env)->CallStaticVoidMethod(env, class_ref, method, id_ref, increment);
    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, id_ref);
}

void ek_achievement_show() {
    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, CLASS_PATH);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "achievement_show", "()V");
    (*env)->CallStaticVoidMethod(env, class_ref, method);
    (*env)->DeleteLocalRef(env, class_ref);
}