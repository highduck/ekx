package ekapp;

import android.content.res.AssetManager;

public class EKPlatform {

    public static native void handle_enter_frame();

    public static native void handle_resume();

    public static native void handle_pause();

    public static native void handle_touch_event(int type, int id, float x, float y);

    public static native void handle_resize(int width, int height, float scaleFactor);

    public static native void on_startup();

    public static native void on_ready();

    public static native void handle_back_button();

    public static native void set_assets_manager(AssetManager assets);
}
