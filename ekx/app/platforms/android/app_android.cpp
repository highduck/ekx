#include <ek/app/Platform.h>
#include <ek/app/app.hpp>

#include <pthread.h>

// implementations
#include "impl/device_android.h"
#include "impl/resources_android.h"
#include "impl/sharing_android.h"
#include "impl/user_prefs_android.h"

namespace ek::app {

struct AndroidApp {
    // Pinned EkActivity class reference
    jclass cls;

    // EK get context static method signature
    jmethodID getContext;

    // EK get activity static method signature
    jmethodID getActivity;

    AAssetManager* assets;
    jobject assetsObject;

    /* pthread key for proper JVM thread handling */
    pthread_key_t threadKey;

    /* Java VM reference */
    JavaVM* jvm;
};

inline AndroidApp androidApp{};

void jniThreadDestructor(void* value) {
    auto* env = static_cast<JNIEnv*>(value);
    if (env) {
        androidApp.jvm->DetachCurrentThread();
        pthread_setspecific(androidApp.threadKey, nullptr);
    }
}

/* Function to retrieve JNI environment, and dealing with threading */
JNIEnv* getJNIEnv() {
    /* Always try to attach if calling from a non-attached thread */
    JNIEnv* env = nullptr;
    if (androidApp.jvm->AttachCurrentThread(&env, nullptr) < 0) {
        EKAPP_LOG("failed to attach current thread");
        return nullptr;
    }
    pthread_setspecific(androidApp.threadKey, (void*) env);
    return env;
}

jobject get_activity() {
    jobject object = getJNIEnv()->CallStaticObjectMethod(androidApp.cls, androidApp.getActivity);
    if (!object) {
        EKAPP_LOG("failed get_activity");
    }
    return object;
}

jobject get_context() {
    jobject object = getJNIEnv()->CallStaticObjectMethod(androidApp.cls, androidApp.getContext);
    if (!object) {
        EKAPP_LOG("failed get_context");
    }
    return object;
}

AAssetManager* get_asset_manager() {
    return androidApp.assets;
}

jobject assetManagerRef() {
    return androidApp.assetsObject;
}

void exitActivity(int code) {
    auto* env = getJNIEnv();
    auto cls = env->FindClass("ek/EkActivity");
    auto method = env->GetStaticMethodID(cls, "app_exit", "(I)V");
    env->CallStaticVoidMethod(cls, method, code);
}

}

extern "C" {

JNIEXPORT void JNICALL Java_ek_EkPlatform_sendEvent(JNIEnv*, jclass, jint eventType) {
    using namespace ek::app;
    processEvent((EventType) eventType);
}

JNIEXPORT void JNICALL
Java_ek_EkPlatform_sendTouch(JNIEnv*, jclass, jint type, jint id, jfloat x, jfloat y) {
    using namespace ek::app;
    const TouchEvent ev{
            static_cast<EventType>(type),
            static_cast<uint64_t>(id),
            x, y
    };
    processEvent(ev);
}

JNIEXPORT void JNICALL
Java_ek_EkPlatform_sendKeyEvent(JNIEnv*, jclass, jint type, jint code, jint modifiers) {
    using namespace ek::app;
    const KeyEvent ev{static_cast<EventType>(type),
                      (KeyCode) code,
                      (KeyModifier) modifiers
    };
    processEvent(ev);
}

JNIEXPORT void JNICALL
Java_ek_EkPlatform_sendResize(JNIEnv*, jclass, jint width, jint height, jfloat scale) {
    using namespace ek::app;
    // currently android always in fullscreen mode
    g_app.fullscreen = true;
    const auto w = (float) width;
    const auto h = (float) height;
    if (g_app.dpiScale != scale ||
        g_app.drawableWidth != w ||
        g_app.drawableHeight != h ||
        g_app.windowWidth != w ||
        g_app.windowHeight != h) {

        g_app.dpiScale = scale;
        g_app.windowWidth = w;
        g_app.windowHeight = h;
        g_app.drawableWidth = w;
        g_app.drawableHeight = h;
        g_app.dirtySize = true;
    }
}

JNIEXPORT int JNICALL Java_ek_EkPlatform_main(JNIEnv*, jclass, jobject assets) {
    using namespace ek::app;
    auto* env = getJNIEnv();
    androidApp.assetsObject = env->NewGlobalRef(assets);
    androidApp.assets = AAssetManager_fromJava(env, androidApp.assetsObject);

    ::ek::app::main();

    // return configuration flags
    return g_app.config.needDepth ? 1 : 0;
}

JNIEXPORT void JNICALL Java_ek_EkPlatform_notifyReady(JNIEnv*, jclass) {
    ek::app::notifyReady();
}

JNIEXPORT void JNICALL Java_ek_EkPlatform_processFrame(JNIEnv*, jclass) {
    using namespace ek::app;
    processFrame();
    if (g_app.exitRequired) {
        exitActivity(g_app.exitCode);
    }
}

/* JNI_OnLoad is automatically called when loading shared library through System.loadLibrary() Java call */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    using namespace ek::app;
    JNIEnv* env;
    jclass cls;

    EKAPP_LOG("JNI_OnLoad BEGIN");

    androidApp.jvm = vm;
    if (androidApp.jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        EKAPP_LOG("Failed to get the environment using GetEnv()");
        return -1;
    }

    /* Create pthread "destructor" pthread key to detach properly all threads */
    if (pthread_key_create(&androidApp.threadKey, jniThreadDestructor) != 0) {
        EKAPP_LOG("Error initializing pthread key");
    }

    /* Make sure we are attached (we should) and setup pthread destructor */
    env = getJNIEnv();

    /* Try to retrieve local reference to our Activity class */
    cls = env->FindClass("ek/EkActivity");
    if (!cls) {
        EKAPP_LOG("Error cannot find Activity class");
    }

    /* Create a global reference for our Activity class */
    androidApp.cls = (jclass) env->NewGlobalRef(cls);

    /* Retrieve the getContext() method id */
    androidApp.getContext = env->GetStaticMethodID(androidApp.cls, "getContext",
                                                   "()Landroid/content/Context;");
    if (!androidApp.getContext) {
        EKAPP_LOG("Error cannot get getContext() method on specified Activity class (not an Activity?)");
    }

    /* Retrieve the getActivity() method id */
    androidApp.getActivity = env->GetStaticMethodID(androidApp.cls, "getActivity", "()Landroid/app/Activity;");
    if (!androidApp.getActivity) {
        EKAPP_LOG("Error cannot get getActivity() method on specified Activity class (not an Activity?)");
    }

    EKAPP_LOG("JNI_OnLoad END");

    return JNI_VERSION_1_6;
}

}