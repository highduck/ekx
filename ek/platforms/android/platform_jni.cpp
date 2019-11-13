#include <jni.h>
#include <platform/application.hpp>
#include "ek/android.hpp"
#include <platform/boot.h>

using namespace ek;

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1enter_1frame(JNIEnv *env, jclass type) {
    g_app.dispatch_draw_frame();
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1resume(JNIEnv *env, jclass type) {
    g_app.dispatch({app_event_type::resumed});
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1pause(JNIEnv *env, jclass type) {
    g_app.dispatch({app_event_type::paused});
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1touch_1event(JNIEnv *env, jclass type_, jint type, jint id,
                                           jfloat x, jfloat y) {

    g_app.dispatch({(touch_event_type) type, (uint64_t) (id + 1), x, y});
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1resize(JNIEnv *env, jclass type, jint width, jint height,
                                     jfloat scale_factor) {

    g_window.device_pixel_ratio = scale_factor;
    g_window.window_size = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
    };
    g_window.back_buffer_size = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
    };
    g_window.size_changed = true;

}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_on_1startup(JNIEnv *env, jclass type) {
    int argc = 0;
    char *argv[0];
    EK_MAIN_ENTRY_POINT(argc, argv);
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_on_1ready(JNIEnv *env, jclass type) {
    g_app.start();
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_handle_1back_1button(JNIEnv *env, jclass type) {
    g_app.dispatch({app_event_type::back_button});
}

extern "C" JNIEXPORT void JNICALL
Java_ekapp_EKPlatform_set_1assets_1manager(JNIEnv *env, jclass type, jobject assets) {

    android::set_asset_manager(assets);
}