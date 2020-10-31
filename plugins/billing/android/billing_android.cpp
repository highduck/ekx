#include <billing.hpp>
#include <ek/android.hpp>
#include <vector>
#include <string>

using namespace ek;

std::string jniGetString(JNIEnv *env, jstring jstr) {
    if (!jstr) {
        return "";
    }
    const char *cstr = env->GetStringUTFChars(jstr, nullptr);
    std::string str = cstr;
    env->ReleaseStringUTFChars(jstr, cstr);
    return str;
}

jobjectArray jniGetObjectStringArray(const std::vector<std::string> &src, JNIEnv *env) {
    jobjectArray res = env->NewObjectArray(src.size(), env->FindClass("java/lang/String"), nullptr);

    for (size_t i = 0; i < src.size(); ++i) {
        env->PushLocalFrame(1);
        env->SetObjectArrayElement(res, i, env->NewStringUTF(src[i].c_str()));
        env->PopLocalFrame(0);
    }

    return res;
}

namespace billing {

    const char *class_path = "ek/billing/BillingBridge";

    void initialize(const std::string &key) {
        auto *env = android::get_jni_env();

        const char *method_name = "initialize";
        const char *method_sig = "(Ljava/lang/String;)V";

        auto class_ref = env->FindClass(class_path);
        auto key_ref = env->NewStringUTF(key.c_str());

        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method, key_ref);

        env->DeleteLocalRef(class_ref);
        env->DeleteLocalRef(key_ref);
    }

    void getPurchases() {
        auto *env = android::get_jni_env();

        const char *method_name = "getPurchases";
        const char *method_sig = "()V";

        auto class_ref = env->FindClass(class_path);
        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method);

        env->DeleteLocalRef(class_ref);
    }

    void getDetails(const std::vector<std::string> &skus) {
        auto *env = android::get_jni_env();

        const char *method_name = "getDetails";
        const char *method_sig = "([Ljava/lang/String;)V";

        auto class_ref = env->FindClass(class_path);
        auto skuList_ref = jniGetObjectStringArray(skus, env);

        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method, skuList_ref);

        env->DeleteLocalRef(class_ref);
        env->DeleteLocalRef(skuList_ref);
    }

    void purchase(const std::string &sku, const std::string &payload) {
        auto *env = android::get_jni_env();

        const char *method_name = "purchase";
        const char *method_sig = "(Ljava/lang/String;Ljava/lang/String;)V";

        auto class_ref = env->FindClass(class_path);
        auto sku_ref = env->NewStringUTF(sku.c_str());
        auto payload_ref = env->NewStringUTF(payload.c_str());

        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method, sku_ref, payload_ref);

        env->DeleteLocalRef(class_ref);
        env->DeleteLocalRef(sku_ref);
        env->DeleteLocalRef(payload_ref);
    }

    void consume(const std::string &token) {
        auto *env = android::get_jni_env();

        const char *method_name = "consume";
        const char *method_sig = "(Ljava/lang/String;)V";

        auto class_ref = env->FindClass(class_path);
        auto token_ref = env->NewStringUTF(token.c_str());

        auto method = env->GetStaticMethodID(class_ref, method_name, method_sig);
        env->CallStaticVoidMethod(class_ref, method, token_ref);

        env->DeleteLocalRef(class_ref);
        env->DeleteLocalRef(class_ref);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_ek_billing_BillingBridge_nativePurchase(JNIEnv *env, jclass clazz,
                                             jstring productID,
                                             jstring token,
                                             jint state,
                                             jstring payload,
                                             jstring signature) {
    using namespace billing;
    PurchaseData data;
    data.productID = jniGetString(env, productID);
    data.token = jniGetString(env, token);
    data.payload = jniGetString(env, payload);
    data.signature = jniGetString(env, signature);
    data.state = state;
    onPurchaseChanged(data);
}

extern "C"
JNIEXPORT void JNICALL
Java_ek_billing_BillingBridge_nativeDetails(JNIEnv *env, jclass clazz,
                                            jstring sku,
                                            jstring price,
                                            jstring currencyCode) {
    using namespace billing;
    ProductDetails data;
    data.sku = jniGetString(env, sku);
    data.price = jniGetString(env, price);
    data.currencyCode = jniGetString(env, currencyCode);
    onProductDetails(data);
}