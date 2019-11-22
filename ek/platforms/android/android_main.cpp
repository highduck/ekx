#include "ek/android.hpp"

#include <platform/application.hpp>
#include <platform/window.hpp>

#include <android/log.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <pthread.h>
#include <string>

const char* JNI_LogTag = "EK:JNI";

/* pthread key for proper JVM thread handling */
static pthread_key_t java_thread_key;

/* Java VM reference */
static JavaVM* java_vm;

/* Main activity */
static jclass java_activity_class;

/* Method signatures */
static jmethodID java_method_id_get_context;
static jmethodID java_method_id_get_activity;


/* Function to retrieve JNI environment, and dealing with threading */
static JNIEnv* android_jni_get_env() {
    /* Always try to attach if calling from a non-attached thread */
    JNIEnv* env = nullptr;
    if (java_vm->AttachCurrentThread(&env, nullptr) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, JNI_LogTag, "failed to attach current thread");
        return nullptr;
    }
    pthread_setspecific(java_thread_key, (void*) env);
    return env;
}

static void android_jni_thread_destructor(void* value) {
    /* The thread is being destroyed, detach it from the Java VM and set the hl_java_thread_key value to NULL as required */
    auto* env = (JNIEnv*) value;
    if (env) {
        java_vm->DetachCurrentThread();
        pthread_setspecific(java_thread_key, nullptr);
    }
}

/* JNI_OnLoad is automatically called when loading shared library through System.loadLibrary() Java call */
EK_JNI_INT(JNI_OnLoad)(JavaVM* vm, void*) {
    JNIEnv* env;
    jclass cls;

    __android_log_print(ANDROID_LOG_INFO, JNI_LogTag, "JNI_OnLoad");

    java_vm = vm;
    if (java_vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        __android_log_print(ANDROID_LOG_ERROR, JNI_LogTag,
                            "Failed to get the environment using GetEnv()");
        return -1;
    }

    /* Create pthread "destructor" pthread key to detach properly all threads */
    if (pthread_key_create(&java_thread_key, android_jni_thread_destructor) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, JNI_LogTag, "Error initializing pthread key");
    }

    /* Make sure we are attached (we should) and setup pthread destructor */
    env = android_jni_get_env();

    /* Try to retrieve local reference to our Activity class */
    cls = env->FindClass("ekapp/EKActivity");
    if (!cls) {
        __android_log_print(ANDROID_LOG_ERROR, JNI_LogTag, "Error cannot find Activity class");
    }

    /* Create a global reference for our Activity class */
    java_activity_class = (jclass) (env->NewGlobalRef(cls));

    /* Retrieve the getContext() method id */
    java_method_id_get_context = env->GetStaticMethodID(java_activity_class, "getContext",
                                                        "()Landroid/content/Context;");
    if (!java_method_id_get_context) {
        __android_log_print(ANDROID_LOG_ERROR, JNI_LogTag,
                            "Error cannot get getContext() method on specified Activity class (not an Activity ?)");
    }

    /* Retrieve the getActivity() method id */
    java_method_id_get_activity = env->GetStaticMethodID(java_activity_class, "getActivity",
                                                         "()Landroid/app/Activity;");
    if (!java_method_id_get_context) {
        __android_log_print(ANDROID_LOG_ERROR, JNI_LogTag,
                            "Error cannot get getActivity() method on specified Activity class (not an Activity ?)");
    }

    __android_log_print(ANDROID_LOG_INFO, JNI_LogTag, "JNI_OnLoad FINISH");
    return JNI_VERSION_1_4;
}

namespace ek {
namespace android {

// Assets Manager global ref
static AAssetManager* asset_manager = nullptr;

JNIEnv* get_jni_env() {
    return android_jni_get_env();
}

jobject get_activity() {
    jobject object = android_jni_get_env()->CallStaticObjectMethod(java_activity_class,
                                                                   java_method_id_get_activity);
    if (!object) {
        __android_log_print(ANDROID_LOG_ERROR, JNI_LogTag, "failed get_activity");
    }
    return object;
}

jobject get_context() {
    jobject object = android_jni_get_env()->CallStaticObjectMethod(java_activity_class,
                                                                   java_method_id_get_context);
    if (!object) {
        __android_log_print(ANDROID_LOG_ERROR, JNI_LogTag, "failed get_context");
    }
    return object;
}

void set_asset_manager(jobject j_asset_manager) {
    asset_manager = AAssetManager_fromJava(get_jni_env(), j_asset_manager);
}

AAssetManager* get_asset_manager() {
    return asset_manager;
}
}

void start_application() {
    g_app.init();

    auto* env = android_jni_get_env();
    auto class_ref = env->FindClass("ekapp/EKActivity");
    auto method_id = env->GetStaticMethodID(class_ref, "start_application", "()V");
    env->CallStaticVoidMethod(class_ref, method_id);
    env->DeleteLocalRef(class_ref);
}

native_window_context_t window_t::view_context() const {
    return android::get_activity();
}

void application_t::exit(int code) {
    auto* env = android_jni_get_env();
    auto class_ref = env->FindClass("ekapp/EKActivity");
    auto method = env->GetStaticMethodID(class_ref, "app_exit", "(I)V");
    env->CallStaticIntMethod(class_ref, method);
}

void window_t::update_mouse_cursor() {}

std::string get_device_lang() {
    auto* env = android_jni_get_env();
    auto class_ref = env->FindClass("ekapp/EKActivity");
    auto method = env->GetStaticMethodID(class_ref, "get_device_lang", "()Ljava/lang/String;");
    auto rv = (jstring) env->CallStaticObjectMethod(class_ref, method);
    const char* temp_str = env->GetStringUTFChars(rv, 0);
    std::string result{temp_str};
    env->ReleaseStringUTFChars(rv, temp_str);
    return result;
}

}

int main() {}