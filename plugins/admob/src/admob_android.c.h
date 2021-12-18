#include <ek/app_native.h>

#define CLASS_PATH "ek/admob/AdMobPlugin"

bool ek_admob_supported(void) {
    return true;
}

void ek_admob_init(ek_admob_config config) {
    ek_admob__init();
    ek_admob.config = config;

    const char* method_name = "initialize";
    const char* method_sig = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V";

    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, CLASS_PATH);
    jstring banner_ref = config.banner ? (*env)->NewStringUTF(env, config.banner) : NULL;
    jstring video_ref = config.video ? (*env)->NewStringUTF(env, config.video) : NULL;
    jstring inters_ref = config.inters ? (*env)->NewStringUTF(env, config.inters) : NULL;

    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, method_name, method_sig);
    (*env)->CallStaticVoidMethod(env, class_ref, method,
                              banner_ref,
                              video_ref,
                              inters_ref,
                              (jint)config.child_directed);

    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, banner_ref);
    (*env)->DeleteLocalRef(env, video_ref);
    (*env)->DeleteLocalRef(env, inters_ref);
}

void ek_admob_show_banner(int flags) {
    const char* method_name = "show_banner";
    const char* method_sig = "(I)V";

    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, CLASS_PATH);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, method_name, method_sig);
    (*env)->CallStaticVoidMethod(env, class_ref, method, flags);
    (*env)->DeleteLocalRef(env, class_ref);
}

void ek_admob_show_rewarded_ad() {
    const char* method_name = "show_rewarded_ad";
    const char* method_sig = "()V";

    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, CLASS_PATH);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, method_name, method_sig);
    (*env)->CallStaticVoidMethod(env, class_ref, method);
    (*env)->DeleteLocalRef(env, class_ref);
}

void ek_admob_show_interstitial_ad() {
    const char* method_name = "show_interstitial_ad";
    const char* method_sig = "()V";

    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, CLASS_PATH);
    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, method_name, method_sig);
    (*env)->CallStaticVoidMethod(env, class_ref, method);
    (*env)->DeleteLocalRef(env, class_ref);
}

JNIEXPORT void JNICALL Java_ek_admob_AdMobPlugin_eventCallback(JNIEnv* env, jclass cls, jint event) {
    (void)env, (void)cls;
    ek_admob__post((ek_admob_event_type)event);
}

