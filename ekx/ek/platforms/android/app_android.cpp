#include <ek/android.hpp>

#include <ek/app/app.hpp>

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <pthread.h>
#include <string>

// implementations
#include "impl/analytics_android.h"
#include "impl/device_android.h"
#include "impl/resources_android.h"
#include "impl/sharing_android.h"
#include "impl/user_prefs_android.h"

using namespace ek::app;

struct android_context {
    // EK bridge class ref
    jclass cls;

    // EK get context static method signature
    jmethodID get_context;

    // EK get activity static method signature
    jmethodID get_activity;

    AAssetManager *assets;
    jobject assetsRef;

    /* pthread key for proper JVM thread handling */
    pthread_key_t thread_key;

    /* Java VM reference */
    JavaVM *jvm;
};

static android_context ctx_;

/* Function to retrieve JNI environment, and dealing with threading */
static JNIEnv *android_jni_get_env() {
    /* Always try to attach if calling from a non-attached thread */
    JNIEnv *env = nullptr;
    if (ctx_.jvm->AttachCurrentThread(&env, nullptr) < 0) {
        EK_ALOGE("failed to attach current thread");
        return nullptr;
    }
    pthread_setspecific(ctx_.thread_key, (void *) env);
    return env;
}

static void android_jni_thread_destructor(void *value) {
    auto *env = static_cast<JNIEnv *>(value);
    if (env) {
        ctx_.jvm->DetachCurrentThread();
        pthread_setspecific(ctx_.thread_key, nullptr);
    }
}

/* JNI_OnLoad is automatically called when loading shared library through System.loadLibrary() Java call */
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    JNIEnv *env;
    jclass cls;

    EK_ALOGI("JNI_OnLoad BEGIN");

    ctx_.jvm = vm;
    if (ctx_.jvm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        EK_ALOGE("Failed to get the environment using GetEnv()");
        return -1;
    }

    /* Create pthread "destructor" pthread key to detach properly all threads */
    if (pthread_key_create(&ctx_.thread_key, android_jni_thread_destructor) != 0) {
        EK_ALOGE("Error initializing pthread key");
    }

    /* Make sure we are attached (we should) and setup pthread destructor */
    env = android_jni_get_env();

    /* Try to retrieve local reference to our Activity class */
    cls = env->FindClass("ek/EkActivity");
    if (!cls) {
        EK_ALOGE("Error cannot find Activity class");
    }

    /* Create a global reference for our Activity class */
    ctx_.cls = (jclass) env->NewGlobalRef(cls);

    /* Retrieve the getContext() method id */
    ctx_.get_context = env->GetStaticMethodID(ctx_.cls, "getContext",
                                              "()Landroid/content/Context;");
    if (!ctx_.get_context) {
        EK_ALOGE("Error cannot get getContext() method on specified Activity class (not an Activity?)");
    }

    /* Retrieve the getActivity() method id */
    ctx_.get_activity = env->GetStaticMethodID(ctx_.cls, "getActivity", "()Landroid/app/Activity;");
    if (!ctx_.get_activity) {
        EK_ALOGE("Error cannot get getActivity() method on specified Activity class (not an Activity?)");
    }

    EK_ALOGI("JNI_OnLoad END");

    return JNI_VERSION_1_6;
}

namespace ek::android {

    JNIEnv *get_jni_env() {
        return android_jni_get_env();
    }

    jobject get_activity() {
        jobject object = android_jni_get_env()->CallStaticObjectMethod(ctx_.cls, ctx_.get_activity);
        if (!object) {
            EK_ALOGE("failed get_activity");
        }
        return object;
    }

    jobject get_context() {
        jobject object = android_jni_get_env()->CallStaticObjectMethod(ctx_.cls, ctx_.get_context);
        if (!object) {
            EK_ALOGE("failed get_context");
        }
        return object;
    }

    AAssetManager *get_asset_manager() {
        return ctx_.assets;
    }

    jobject assetManagerRef() {
        return ctx_.assetsRef;
    }

    void exit_activity(int code) {
        auto *env = android_jni_get_env();
        auto class_ref = env->FindClass("ek/EkActivity");
        auto method = env->GetStaticMethodID(class_ref, "app_exit", "(I)V");
        env->CallStaticVoidMethod(class_ref, method, code);
    }
}

namespace ek {
    void start_application() {
        int flags = g_app.window_cfg.needDepth ? 1 : 0;

        dispatch_init();
        auto *env = android_jni_get_env();
        auto class_ref = env->FindClass("ek/EkActivity");
        auto method_id = env->GetStaticMethodID(class_ref, "start_application", "(I)V");
        env->CallStaticVoidMethod(class_ref, method_id, flags);
        env->DeleteLocalRef(class_ref);
    }
}

void handle_enter_frame() {

    dispatch_draw_frame();

    if (g_app.require_exit) {
        ek::android::exit_activity(g_app.exit_code);
    }
}

constexpr uint8_t event_nop = 0xFF;
constexpr uint8_t call_dispatch_frame = 0x70;
constexpr uint8_t call_start = 0x71;
constexpr uint8_t call_ready = 0x72;

const uint8_t event_map_[17] = {
        call_dispatch_frame, // 0 - dispatch frame
        call_start, // 1
        call_ready, // 2
        event_nop, // 3
        event_nop, // 4
        event_nop, // 5
        event_nop, // 6
        event_nop, // 7
        static_cast<uint8_t>(event_type::key_down), // 8
        static_cast<uint8_t>(event_type::key_up), // 9

        static_cast<uint8_t>(event_type::app_resume), // 10
        static_cast<uint8_t>(event_type::app_pause),
        static_cast<uint8_t>(event_type::app_back_button),

        static_cast<uint8_t>(event_type::touch_begin), // 13
        static_cast<uint8_t>(event_type::touch_move),
        static_cast<uint8_t>(event_type::touch_end), // 15
        event_nop // 16
};

EK_JNI_VOID(sendEvent, jint type) {
    assert(type >= 0 && type < 17);
    const uint8_t event_type_id = event_map_[type];
    switch (event_type_id) {
        case call_dispatch_frame:
            handle_enter_frame();
            break;
        case call_start:
            ::ek::main();
            break;
        case call_ready:
            dispatch_device_ready();
            break;
        case event_nop:
            break;
        default:
            dispatch_event({static_cast<event_type>(event_type_id)});
            break;
    }
}

EK_JNI_VOID(sendTouch, jint type, jint id, jfloat x, jfloat y) {
    assert(type >= 0 && type < 17);

    uint8_t event_type_id = event_map_[type];
    event_t ev{static_cast<event_type>(event_type_id)};
    ev.id = static_cast<uint64_t>(id) + 1;
    ev.pos.x = x;
    ev.pos.y = y;
    dispatch_event(ev);
}

EK_JNI_VOID(sendKeyEvent, jint type, jint code, jint modifiers) {
    assert(type >= 0 && type < 17);

    uint8_t event_type_id = event_map_[type];
    event_t ev{static_cast<event_type>(event_type_id)};
    ev.code = static_cast<key_code>(code);
    // TODO: modifiers

    dispatch_event(ev);
}

EK_JNI_VOID(sendResize, jint width, jint height, jfloat scale) {
    // currently android always in fullscreen mode
    g_app.fullscreen = true;
    if (g_app.content_scale != scale ||
        g_app.drawable_size.x != width ||
        g_app.drawable_size.y != height ||
        g_app.window_size.x != width ||
        g_app.window_size.y != height) {

        g_app.content_scale = scale;
        g_app.window_size.x = width;
        g_app.window_size.y = height;
        g_app.drawable_size.x = width;
        g_app.drawable_size.y = height;
        g_app.size_changed = true;
    }
}

EK_JNI_VOID(initAssets, jobject assets) {
    auto* env_ = android_jni_get_env();
    ctx_.assetsRef = env_->NewGlobalRef(assets);
    ctx_.assets = AAssetManager_fromJava(env_, ctx_.assetsRef);
}