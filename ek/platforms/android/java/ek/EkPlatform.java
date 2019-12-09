package ek;

import android.content.res.AssetManager;

public class EkPlatform {

    static {
        System.loadLibrary("native-lib");
    }

    static final int CALL_DISPATCH_FRAME = 0;
    static final int CALL_START = 1;
    static final int CALL_DEVICE_READY = 2;

    static final int RESUME = 10;
    static final int PAUSE = 11;
    static final int BACK_BUTTON = 12;

    static final int TOUCH_BEGIN = 13;
    static final int TOUCH_MOVE = 14;
    static final int TOUCH_END = 15;

    public static native void sendEvent(int type);

    public static native void sendTouch(int type, int id, float x, float y);

    public static native void sendResize(int width, int height, float scaleFactor);

    public static native void initAssets(AssetManager assets);
}
