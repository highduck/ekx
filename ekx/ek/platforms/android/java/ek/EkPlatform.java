package ek;

import android.content.res.AssetManager;

import androidx.annotation.Keep;

@Keep
public class EkPlatform {

    static final int CALL_DISPATCH_FRAME = 0;
    static final int CALL_START = 1;
    static final int CALL_DEVICE_READY = 2;

    static final int KEY_DOWN = 8;
    static final int KEY_UP = 9;

    static final int RESUME = 10;
    static final int PAUSE = 11;
    static final int BACK_BUTTON = 12;

    static final int TOUCH_BEGIN = 13;
    static final int TOUCH_MOVE = 14;
    static final int TOUCH_END = 15;

    @Keep
    public static native void sendEvent(int type);

    @Keep
    public static native void sendTouch(int type, int id, float x, float y);

    @Keep
    public static native void sendKeyEvent(int type, int code, int modifiers);

    @Keep
    public static native void sendResize(int width, int height, float scaleFactor);

    @Keep
    public static native void initAssets(AssetManager assets);
}
