#include <ek/app_native.h>
#include <pthread.h>

static struct {
    /* Java VM reference */
    JavaVM* jvm;

    AAssetManager* assets;

    // Pinned EkActivity class reference
    jclass cls;

    // EK get context static method signature
    jmethodID getContext;

    // EK get activity static method signature
    jmethodID getActivity;

    jobject assetsObject;

    /* pthread key for proper JVM thread handling */
    pthread_key_t thread_key;
} android_app;

static void jni_thread_destructor(void* value) {
    JNIEnv* env = (JNIEnv*) value;
    JavaVM* jvm = android_app.jvm;
    if (env && jvm && *jvm) {
        (*jvm)->DetachCurrentThread(jvm);
        pthread_setspecific(android_app.thread_key, NULL);
    }
}

/* Function to retrieve JNI environment, and dealing with threading */
JNIEnv* ek_android_jni(void) {
    /* Always try to attach if calling from a non-attached thread */
    JNIEnv* env = NULL;
    JavaVM* jvm = android_app.jvm;
    if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) < 0) {
        log_debug("failed to attach current thread");
        return NULL;
    }
    pthread_setspecific(android_app.thread_key, env);
    return env;
}

jobject ek_android_activity(void) {
    JNIEnv* env = ek_android_jni();
    jobject object = (*env)->CallStaticObjectMethod(env, android_app.cls, android_app.getActivity);
    if (!object) {
        log_debug("failed get_activity");
    }
    return object;
}

jobject ek_android_context(void) {
    JNIEnv* env = ek_android_jni();
    jobject object = (*env)->CallStaticObjectMethod(env, android_app.cls, android_app.getContext);
    if (!object) {
        log_debug("failed get_context");
    }
    return object;
}

AAssetManager* ek_android_assets() {
    return android_app.assets;
}

jobject ek_android_assets_object() {
    return android_app.assetsObject;
}

static void activity_exit(int code) {
    JNIEnv* env = ek_android_jni();
    jclass cls = (*env)->FindClass(env, "ek/EkActivity");
    jmethodID method = (*env)->GetStaticMethodID(env, cls, "app_exit", "(I)V");
    (*env)->CallStaticVoidMethod(env, cls, method, code);
}

JNIEXPORT void JNICALL Java_ek_EkPlatform_sendEvent(JNIEnv* env_, jclass cls_, jint eventType) {
    (void)sizeof(env_);
    (void)sizeof(cls_);
    ek_app__process_event((ek_app_event) {.type= (ek_app_event_type) eventType});
}

JNIEXPORT void JNICALL
Java_ek_EkPlatform_sendTouch(JNIEnv* env_, jclass cls_, jint type, jint id, jfloat x, jfloat y) {
    (void)sizeof(env_);
    (void)sizeof(cls_);
    ek_app__process_event((ek_app_event) {.touch= {
            .type =    (ek_app_event_type) type,
            .id =   (uint64_t) id,
            .x = x,
            .y = y
    }});
}

JNIEXPORT void JNICALL
Java_ek_EkPlatform_sendKeyEvent(JNIEnv* env_, jclass cls_, jint type, jint code, jint modifiers) {
    (void)sizeof(env_);
    (void)sizeof(cls_);
    ek_app__process_event((ek_app_event) {.key = {
            .type = (ek_app_event_type) type,
            .code = (ek_key_code) code,
            .modifiers = (ek_key_mod) modifiers
    }});
}

int ek_app_open_url(const char* url) {
    JNIEnv* env = ek_android_jni();
    jstring jURL = (*env)->NewStringUTF(env, url);
    jclass cls = (*env)->FindClass(env, "ek/EkDevice");

    jmethodID method = (*env)->GetStaticMethodID(env, cls, "openURL", "(Ljava/lang/String;)I");
    int result = (*env)->CallStaticIntMethod(env, cls, method, jURL);

    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, jURL);
    return result;
}

static void ek_app__init_lang(void) {
    JNIEnv* env = ek_android_jni();
    jclass cls = (*env)->FindClass(env, "ek/EkDevice");
    jmethodID method = (*env)->GetStaticMethodID(env, cls, "getLanguage", "()Ljava/lang/String;");
    jstring rv = (jstring) (*env)->CallStaticObjectMethod(env, cls, method);
    const char* temp = (*env)->GetStringUTFChars(env, rv, NULL);
    ek_app.lang[0] = '\0';
    strncat(ek_app.lang, temp, 7);
    (*env)->ReleaseStringUTFChars(env, rv, temp);
}

int ek_app_font_path(char* dest, uint32_t size, const char* font_name) {
    (void)font_name;
    dest[0] = '\0';
    //"/system/fonts/Roboto-Regular.ttf";
    strncat(dest, "/system/fonts/DroidSansFallback.ttf", size);
    return 0;
}

int ek_app_share(const char* content) {
    JNIEnv* env = ek_android_jni();
    jstring jContent = (*env)->NewStringUTF(env, content);
    jclass cls = (*env)->FindClass(env, "ek/EkDevice");

    jmethodID method = (*env)->GetStaticMethodID(env, cls, "share", "(Ljava/lang/String;)I");
    int result = (*env)->CallStaticIntMethod(env, cls, method, jContent);

    (*env)->DeleteLocalRef(env, cls);
    (*env)->DeleteLocalRef(env, jContent);
    return result;
}

JNIEXPORT void JNICALL
Java_ek_EkPlatform_sendResize(JNIEnv* env, jclass cls_, jfloatArray values) {
    (void)sizeof(cls_);
    const jfloat* floatValues = (*env)->GetFloatArrayElements(env, values, NULL);
    ek_app__update_viewport((ek_app_viewport) {
            .width = floatValues[0],
            .height = floatValues[1],
            .scale = floatValues[2],
            .insets[0] = floatValues[3],
            .insets[1] = floatValues[4],
            .insets[2] = floatValues[5],
            .insets[3] = floatValues[6],
    });
}

JNIEXPORT int JNICALL Java_ek_EkPlatform_main(JNIEnv* env_, jclass cls_, jobject assets) {
    (void)sizeof(env_);
    (void)sizeof(cls_);
    JNIEnv* env = ek_android_jni();
    android_app.assetsObject = (*env)->NewGlobalRef(env, assets);
    android_app.assets = AAssetManager_fromJava(env, android_app.assetsObject);

    ek_app__init();
    ek_app.state |= EK_APP_STATE_FULLSCREEN;
    ek_app__init_lang();

    ek_app_main();

    if (ek_app.state & EK_APP_STATE_EXIT_PENDING) {
        activity_exit(ek_app.exit_code);
    }

// return configuration flags
    return ek_app.config.need_depth ? 1 : 0;
}

JNIEXPORT void JNICALL Java_ek_EkPlatform_notifyReady(JNIEnv* env_, jclass cls_) {
    (void)sizeof(env_);
    (void)sizeof(cls_);
    ek_app__notify_ready();
}

JNIEXPORT void JNICALL Java_ek_EkPlatform_processFrame(JNIEnv* env_, jclass cls_) {
    (void)sizeof(env_);
    (void)sizeof(cls_);
    ek_app__process_frame();
    if (ek_app.state & EK_APP_STATE_EXIT_PENDING) {
        activity_exit(ek_app.exit_code);
    }
}

/* JNI_OnLoad is automatically called when loading shared library through System.loadLibrary() Java call */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* _) {
    (void)sizeof(_);

    JNIEnv* env;
    jclass cls;

    log_debug("JNI_OnLoad BEGIN");

    android_app.jvm = vm;
    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        log_debug("Failed to get the environment using GetEnv()");
        return -1;
    }

    /* Create pthread "destructor" pthread key to detach properly all threads */
    if (pthread_key_create(&android_app.thread_key, jni_thread_destructor) != 0) {
        log_debug("Error initializing pthread key");
    }

    /* Make sure we are attached (we should) and setup pthread destructor */
    env = ek_android_jni();

    /* Try to retrieve local reference to our Activity class */
    cls = (*env)->FindClass(env, "ek/EkActivity");
    if (!cls) {
        log_debug("Error cannot find Activity class");
    }

    /* Create a global reference for our Activity class */
    android_app.cls = (jclass) (*env)->NewGlobalRef(env, cls);

    /* Retrieve the getContext() method id */
    android_app.getContext = (*env)->GetStaticMethodID(env,
                                                       android_app.cls,
                                                       "getContext",
                                                       "()Landroid/content/Context;");
    if (!android_app.getContext) {
        log_debug("Error cannot get getContext() method on specified Activity class (not an Activity?)");
    }

    /* Retrieve the getActivity() method id */
    android_app.getActivity = (*env)->GetStaticMethodID(env,
                                                        android_app.cls,
                                                        "getActivity",
                                                        "()Landroid/app/Activity;");
    if (!android_app.getActivity) {
        log_debug("Error cannot get getActivity() method on specified Activity class (not an Activity?)");
    }

    log_debug("JNI_OnLoad END");

    return JNI_VERSION_1_6;
}
