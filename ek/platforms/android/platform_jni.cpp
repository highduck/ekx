#include <platform/application.hpp>
#include <platform/boot.hpp>
#include <ek/android.hpp>

using namespace ek;

EK_JNI(Java_ekapp_EKPlatform_handle_1enter_1frame)(JNIEnv*, jclass) {
    g_app.dispatch_draw_frame();
}

EK_JNI(Java_ekapp_EKPlatform_handle_1resume)(JNIEnv*, jclass) {
    g_app.dispatch({event_type::app_resume});
}

EK_JNI(Java_ekapp_EKPlatform_handle_1pause)(JNIEnv*, jclass) {
    g_app.dispatch({event_type::app_pause});
}

const event_type touch_event_map_[3]{
        event_type::touch_begin,
        event_type::touch_move,
        event_type::touch_end
};

EK_JNI(Java_ekapp_EKPlatform_handle_1touch_1event)(JNIEnv*, jclass, jint touch_type, jint id, jfloat x, jfloat y) {
    event_t ev{touch_event_map_[touch_type]};
    ev.id = (uint64_t) id + 1;
    ev.set_position(x, y);
    g_app.dispatch(ev);
}

EK_JNI(Java_ekapp_EKPlatform_handle_1resize)(JNIEnv*, jclass, jint width, jint height, jfloat scale_factor) {
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

EK_JNI(Java_ekapp_EKPlatform_on_1startup)(JNIEnv*, jclass) {
    EK_MAIN_ENTRY_POINT(0, nullptr);
}

EK_JNI(Java_ekapp_EKPlatform_on_1ready)(JNIEnv*, jclass) {
    g_app.start();
}

EK_JNI(Java_ekapp_EKPlatform_handle_1back_1button)(JNIEnv*, jclass) {
    g_app.dispatch({event_type::app_back_button});
}

EK_JNI(Java_ekapp_EKPlatform_set_1assets_1manager)(JNIEnv*, jclass, jobject assets) {
    android::set_asset_manager(assets);
}