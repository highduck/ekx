#include <ek/app_native.h>

void ek_ls_set_s(const char* key, const char* str) {
    JNIEnv* env = ek_android_jni();

    jclass class_ref = (*env)->FindClass(env, "ek/LocalStorage");
    jstring key_ref = (*env)->NewStringUTF(env, key);
    jstring val_ref = (*env)->NewStringUTF(env, str);

    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "set_string", "(Ljava/lang/String;Ljava/lang/String;)V");
    (*env)->CallStaticVoidMethod(env, class_ref, method, key_ref, val_ref);

    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, key_ref);
    (*env)->DeleteLocalRef(env, val_ref);
}

int ek_ls_get_s(const char* key, char* buffer, uint32_t buffer_size) {
    JNIEnv* env = ek_android_jni();
    jclass class_ref = (*env)->FindClass(env, "ek/LocalStorage");
    jstring key_ref = (*env)->NewStringUTF(env, key);

    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "get_string", "(Ljava/lang/String;)Ljava/lang/String;");
    jobject result = (jstring)(*env)->CallStaticObjectMethod(env, class_ref, method, key_ref);

    jboolean is_copy;
    const char* data = (*env)->GetStringUTFChars(env, result, &is_copy);
    uint32_t size = (*env)->GetStringUTFLength(env, result);
    if(size >= buffer_size) {
      size = buffer_size - 1;
    }
    memcpy(buffer, data, size);
    buffer[size + 1] = 0;

    (*env)->ReleaseStringUTFChars(env, result, data);
    (*env)->DeleteLocalRef(env, result);

    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, key_ref);

    return size;
}

void ek_ls_set_i(const char* key, int value) {
    JNIEnv* env = ek_android_jni();

    jclass class_ref = (*env)->FindClass(env, "ek/LocalStorage");
    jstring key_ref = (*env)->NewStringUTF(env, key);

    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "set_int", "(Ljava/lang/String;I)V");
    (*env)->CallStaticVoidMethod(env, class_ref, method, key_ref, value);

    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, key_ref);
}

int ek_ls_get_i(const char* key, int de_fault) {
    JNIEnv* env = ek_android_jni();

    jclass class_ref = (*env)->FindClass(env, "ek/LocalStorage");
    jstring key_ref = (*env)->NewStringUTF(env, key);

    jmethodID method = (*env)->GetStaticMethodID(env, class_ref, "get_int", "(Ljava/lang/String;I)I");
    jint result = (*env)->CallStaticIntMethod(env, class_ref, method, key_ref, de_fault);

    (*env)->DeleteLocalRef(env, class_ref);
    (*env)->DeleteLocalRef(env, key_ref);
    return result;
}
