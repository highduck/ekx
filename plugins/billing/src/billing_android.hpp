#pragma once

#include <ek/app_native.h>
#include <ek/ds/Array.hpp>
#include <ek/ds/String.hpp>

ek::String jniGetString(JNIEnv* env, jstring jstr) {
    if (!jstr) {
        return "";
    }
    const char* cstr = env->GetStringUTFChars(jstr, nullptr);
    ek::String str = cstr;
    env->ReleaseStringUTFChars(jstr, cstr);
    return str;
}

jobjectArray jniGetObjectStringArray(const ek::Array<ek::String>& src, JNIEnv* env) {
    jobjectArray res = env->NewObjectArray(src.size(), env->FindClass("java/lang/String"), nullptr);

    for (size_t i = 0; i < src.size(); ++i) {
        env->PushLocalFrame(1);
        env->SetObjectArrayElement(res, i, env->NewStringUTF(src[i].c_str()));
        env->PopLocalFrame(0);
    }

    return res;
}

namespace billing {

const char* class_path = "ek/billing/BillingBridge";

void initialize(const char* key) {
    _initialize();

    auto* env = ek_android_jni();

    const char* method_name = "initialize";
    const char* method_sig = "(Ljava/lang/String;)V";

    auto class_ref = env->FindClass(class_path);
    auto key_ref = env->NewStringUTF(key);

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method, key_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(key_ref);
}

void getPurchases() {
    auto* env = ek_android_jni();

    const char* method_name = "getPurchases";
    const char* method_sig = "()V";

    auto class_ref = env->FindClass(class_path);
    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method);

    env->DeleteLocalRef(class_ref);
}

void getDetails(const ek::Array<ek::String>& skus) {
    auto* env = ek_android_jni();

    const char* method_name = "getDetails";
    const char* method_sig = "([Ljava/lang/String;)V";

    auto class_ref = env->FindClass(class_path);
    auto skuList_ref = jniGetObjectStringArray(skus, env);

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method, skuList_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(skuList_ref);
}

void purchase(const ek::String& sku, const ek::String& payload) {
    auto* env = ek_android_jni();

    const char* method_name = "purchase";
    const char* method_sig = "(Ljava/lang/String;Ljava/lang/String;)V";

    auto class_ref = env->FindClass(class_path);
    auto sku_ref = env->NewStringUTF(sku.c_str());
    auto payload_ref = env->NewStringUTF(payload.c_str());

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method, sku_ref, payload_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(sku_ref);
    env->DeleteLocalRef(payload_ref);
}

void consume(const ek::String& token) {
    auto* env = ek_android_jni();

    const char* method_name = "consume";
    const char* method_sig = "(Ljava/lang/String;)V";

    auto class_ref = env->FindClass(class_path);
    auto token_ref = env->NewStringUTF(token.c_str());

    auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
    env->CallStaticVoidMethod(class_ref, method, token_ref);

    env->DeleteLocalRef(class_ref);
    env->DeleteLocalRef(class_ref);
}
}

extern "C" {

JNIEXPORT void JNICALL Java_ek_billing_BillingBridge_nativePurchase(JNIEnv* env, jclass,
                                                                    jstring productID,
                                                                    jstring token,
                                                                    jint state,
                                                                    jstring payload,
                                                                    jstring signature,
                                                                    jint responseCode) {
    using namespace billing;
    PurchaseData data;
    data.productID = jniGetString(env, productID);
    data.token = jniGetString(env, token);
    data.payload = jniGetString(env, payload);
    data.signature = jniGetString(env, signature);
    data.state = state;
    data.errorCode = responseCode;
    context.onPurchaseChanged(data);
}

JNIEXPORT void JNICALL Java_ek_billing_BillingBridge_nativeDetails(JNIEnv* env, jclass,
                                                                   jstring sku,
                                                                   jstring price,
                                                                   jstring currencyCode) {
    using namespace billing;
    ProductDetails data;
    data.sku = jniGetString(env, sku);
    data.price = jniGetString(env, price);
    data.currencyCode = jniGetString(env, currencyCode);
    context.onProductDetails(data);
}

}