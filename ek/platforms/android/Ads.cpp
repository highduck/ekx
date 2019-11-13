#include <platform/Ads.h>
#include "ek/android.hpp"

namespace ek {

    void ads_reset_purchase() {
        auto *env = android::get_jni_env();

        const char *class_path = "ekapp/Ads";
        const char *method_name = "reset_purchase";
        const char *method_sig = "()V";

        auto class_ref = env->FindClass(class_path);
        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method);

        env->DeleteLocalRef(class_ref);
    }

    void ads_init(const ads_config_t &config) {
        auto *env = android::get_jni_env();

        const char *class_path = "ekapp/Ads";
        const char *method_name = "init";
        const char *method_sig = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V";

        auto class_ref = env->FindClass(class_path);
        auto app_id_ref = env->NewStringUTF(config.app_id.c_str());
        auto banner_ref = env->NewStringUTF(config.banner.c_str());
        auto video_ref = env->NewStringUTF(config.video.c_str());
        auto inters_ref = env->NewStringUTF(config.inters.c_str());
        auto remove_ads_sku_ref = env->NewStringUTF(config.remove_ads_sku.c_str());

        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method,
                                  app_id_ref,
                                  banner_ref,
                                  video_ref,
                                  inters_ref,
                                  remove_ads_sku_ref);

        env->DeleteLocalRef(class_ref);
        env->DeleteLocalRef(app_id_ref);
        env->DeleteLocalRef(banner_ref);
        env->DeleteLocalRef(video_ref);
        env->DeleteLocalRef(inters_ref);
        env->DeleteLocalRef(remove_ads_sku_ref);
    }

    void ads_set_banner(int flags) {
        auto *env = android::get_jni_env();

        const char *class_path = "ekapp/Ads";
        const char *method_name = "set_banner";
        const char *method_sig = "(I)V";

        auto class_ref = env->FindClass(class_path);

        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method, flags);

        env->DeleteLocalRef(class_ref);
    }

    void ads_play_reward_video() {
        auto *env = android::get_jni_env();

        const char *class_path = "ekapp/Ads";
        const char *method_name = "play_reward_video";
        const char *method_sig = "()V";

        auto class_ref = env->FindClass(class_path);

        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method);

        env->DeleteLocalRef(class_ref);
    }

    void ads_show_interstitial() {
        auto *env = android::get_jni_env();

        const char *class_path = "ekapp/Ads";
        const char *method_name = "show_interstitial";
        const char *method_sig = "()V";

        auto class_ref = env->FindClass(class_path);

        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method);

        env->DeleteLocalRef(class_ref);
    }

    // TODO: signals to core module
    static std::function<void(AdsEventType type)> ads_registered_callbacks;

    void ads_listen(const std::function<void(AdsEventType type)> &callback) {
        ads_registered_callbacks = callback;
    }


    void ads_purchase_remove() {
        auto *env = android::get_jni_env();

        const char *class_path = "ekapp/Ads";
        const char *method_name = "purchase_remove";
        const char *method_sig = "()V";

        auto class_ref = env->FindClass(class_path);

        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method);

        env->DeleteLocalRef(class_ref);
    }

    void init_billing(const char *key) {
        auto *env = android::get_jni_env();

        const char *class_path = "ekapp/RemoveAds";
        const char *method_name = "init";
        const char *method_sig = "(Ljava/lang/String;)V";
        auto key_ref = env->NewStringUTF(key);

        auto class_ref = env->FindClass(class_path);
        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method, key_ref);

        env->DeleteLocalRef(class_ref);
        env->DeleteLocalRef(key_ref);
    }

}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_Ads_ads_1event_1callback(JNIEnv *env, jclass type, jint event) {
    using ::ek::ads_registered_callbacks;
    using ::ek::AdsEventType;
    if (ads_registered_callbacks) {
        ads_registered_callbacks(static_cast<AdsEventType>(event));
    }
}