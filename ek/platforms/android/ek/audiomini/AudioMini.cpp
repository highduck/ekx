#include <ek/audiomini/AudioMini.h>
#include <platform/ek_android.h>

namespace ek {

AudioMini::AudioMini() = default;

AudioMini::~AudioMini() = default;

void AudioMini::create_sound(const char* name) {
    if (!name) {
        return;
    }
    auto* env = android::get_jni_env();
    auto j_name = env->NewStringUTF(name);
    auto clazz = env->FindClass("ekapp/EKAudio");
    auto method = env->GetStaticMethodID(clazz, "createSound", "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(clazz, method, j_name);
}

void AudioMini::create_music(const char* name) {
    if (!name) {
        return;
    }
    auto* env = android::get_jni_env();
    auto j_name = env->NewStringUTF(name);
    auto clazz = env->FindClass("ekapp/EKAudio");
    auto method = env->GetStaticMethodID(clazz, "createMusic", "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(clazz, method, j_name);
}

void AudioMini::play_sound(const char* name, float vol, float pan) {
    if (!name) {
        return;
    }

    auto* env = android::get_jni_env();
    auto j_name = env->NewStringUTF(name);
    auto clazz = env->FindClass("ekapp/EKAudio");
    auto method = env->GetStaticMethodID(clazz, "playSound", "(Ljava/lang/String;FF)V");
    env->CallStaticVoidMethod(clazz, method, j_name, vol, pan);
}

void AudioMini::play_music(const char* name, float vol) {
    if (!name) {
        return;
    }

    auto* env = android::get_jni_env();
    auto j_name = env->NewStringUTF(name);
    auto clazz = env->FindClass("ekapp/EKAudio");
    auto method = env->GetStaticMethodID(clazz, "playMusic", "(Ljava/lang/String;F)V");
    env->CallStaticVoidMethod(clazz, method, j_name, vol);
}

void AudioMini::vibrate(int duration_millis) {
    if (duration_millis <= 0) {
        return;
    }
    auto* env = android::get_jni_env();
    auto clazz = env->FindClass("ekapp/EKAudio");
    auto method = env->GetStaticMethodID(clazz, "vibrate", "(J)V");
    env->CallStaticVoidMethod(clazz, method, (jlong)duration_millis);
}

}