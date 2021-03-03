#include <admob.hpp>
#include <ek/android.hpp>

using namespace ek;

namespace admob {

const char *class_path = "ek/admob/AdMobPlugin";

void initialize(const config_t &config) {
    _initialize();

    auto *env = android::get_jni_env();

    const char *method_name = "initialize";
    const char *method_sig = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V";

    auto class_ref = env->FindClass(class_path);
    auto banner_ref = env->NewStringUTF(config.banner.c_str());
    auto video_ref = env->NewStringUTF(config.video.c_str());
    auto inters_ref = env->NewStringUTF(config.inters.c_str());

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method,
                              banner_ref,
                              video_ref,
                              inters_ref,
                              static_cast<int>(config.childDirected));

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(banner_ref);
    env->DeleteLocalRef(video_ref);
    env->DeleteLocalRef(inters_ref);
}

void show_banner(int flags) {
    auto *env = android::get_jni_env();

    const char *method_name = "show_banner";
    const char *method_sig = "(I)V";

    auto class_ref = env->FindClass(class_path);

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method, flags);

    env->DeleteLocalRef(class_ref);
}

void show_rewarded_ad() {
    auto *env = android::get_jni_env();

    const char *method_name = "show_rewarded_ad";
    const char *method_sig = "()V";

    auto class_ref = env->FindClass(class_path);

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method);

    env->DeleteLocalRef(class_ref);
}

void show_interstitial_ad() {
    auto *env = android::get_jni_env();

    const char *method_name = "show_interstitial_ad";
    const char *method_sig = "()V";

    auto class_ref = env->FindClass(class_path);

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method);

    env->DeleteLocalRef(class_ref);
}

bool hasSupport() {
    return true;
}

}

extern "C"
JNIEXPORT void JNICALL Java_ek_admob_AdMobPlugin_eventCallback(JNIEnv *env, jclass type, jint event) {
    using namespace admob;
    if(event == 6) {
        onInterstitialClosed();
    }
    else {
        onEvent(static_cast<event_type>(event));
    }
}