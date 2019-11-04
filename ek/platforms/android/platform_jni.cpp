#include <jni.h>
#include <platform/Application.h>
#include <platform/Window.h>
#include <platform/ek_android.h>
#include <platform/boot.h>

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1enter_1frame(JNIEnv *env, jclass type) {
    ek::gApp.dispatchDrawFrame();
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1resume(JNIEnv *env, jclass type) {

    ek::gApp.dispatch({ek::AppEvent::Type::Resumed});

}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1pause(JNIEnv *env, jclass type) {

    ek::gApp.dispatch({ek::AppEvent::Type::Paused});

}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1touch_1event(JNIEnv *env, jclass type_, jint type, jint id, jfloat x,
                                           jfloat y) {

    ek::gApp.dispatch({(ek::TouchEvent::Type) type, (uint64_t) (id + 1), x, y});

}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1resize(JNIEnv *env, jclass type, jint width, jint height,
                                     jfloat scaleFactor) {

    ek::gWindow.scaleFactor = scaleFactor;
    ek::gWindow.windowSize = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
    };
    ek::gWindow.backBufferSize = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
    };
    ek::gWindow.sizeChanged = true;

}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_on_1startup(JNIEnv *env, jclass type) {
    int argc = 0;
    char *argv[0];
    EK_MAIN_ENTRY_POINT(argc, argv);
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_on_1ready(JNIEnv *env, jclass type) {
    ek::gApp.start();
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1back_1button(JNIEnv *env, jclass type) {
    ek::gApp.dispatch({ek::AppEvent::Type::BackButton});
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_set_1assets_1manager(JNIEnv *env, jclass type, jobject assets) {

    ek::android::set_asset_manager(assets);
}