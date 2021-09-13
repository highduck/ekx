package ek;

import android.content.res.AssetManager;
import android.view.KeyEvent;

import androidx.annotation.Keep;

@Keep
public class EkPlatform {

    // KeyModifier
    static final int KEY_MODIFIER_SUPER = 1;
    static final int KEY_MODIFIER_SHIFT = 2;
    static final int KEY_MODIFIER_CONTROL = 4;
    static final int KEY_MODIFIER_ALT = 8;

    // Special commands to native app
    static final int CALL_DISPATCH_FRAME = 0x100;
    static final int CALL_START = 0x101;
    static final int CALL_DEVICE_READY = 0x102;

    // Event::Type
    static final int RESUME = 0;
    static final int PAUSE = 1;
    static final int RESIZE = 2;
    static final int BACK_BUTTON = 3;
    static final int CLOSE = 4;
    static final int TOUCH_START = 5;
    static final int TOUCH_MOVE = 6;
    static final int TOUCH_END = 7;
    static final int MOUSE_MOVE = 8;
    static final int MOUSE_DOWN = 9;
    static final int MOUSE_UP = 10;
    static final int MOUSE_ENTER = 11;
    static final int MOUSE_EXIT = 12;
    static final int MOUSE_SCROLL = 13;
    static final int KEY_DOWN = 14;
    static final int KEY_UP = 15;
    static final int KEY_PRESS = 16;
    static final int TEXT = 17;

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

    /** converters **/
    public static int _KeyCode(final int keyCode) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_DPAD_UP: return 1;
            case KeyEvent.KEYCODE_DPAD_DOWN: return 2;
            case KeyEvent.KEYCODE_DPAD_LEFT: return 3;
            case KeyEvent.KEYCODE_DPAD_RIGHT: return 4;
            case KeyEvent.KEYCODE_ESCAPE: return 5;
            case KeyEvent.KEYCODE_SPACE: return 6;
            case KeyEvent.KEYCODE_ENTER: return 7;
            case KeyEvent.KEYCODE_FORWARD_DEL: return 8;
            case KeyEvent.KEYCODE_TAB: return 9;
            case KeyEvent.KEYCODE_PAGE_UP: return 10;
            case KeyEvent.KEYCODE_PAGE_DOWN: return 11;
            case KeyEvent.KEYCODE_HOME: return 12;
            case KeyEvent.KEYCODE_MOVE_END: return 13;
            case KeyEvent.KEYCODE_INSERT: return 14;
            case KeyEvent.KEYCODE_DEL: return 15;
            case KeyEvent.KEYCODE_A: return 16;
            case KeyEvent.KEYCODE_C: return 17;
            case KeyEvent.KEYCODE_V: return 18;
            case KeyEvent.KEYCODE_X: return 19;
            case KeyEvent.KEYCODE_Y: return 20;
            case KeyEvent.KEYCODE_Z: return 21;
            case KeyEvent.KEYCODE_W: return 22;
            case KeyEvent.KEYCODE_S: return 23;
            case KeyEvent.KEYCODE_D: return 24;
        }
        return 0;
    }

    public static int _KeyModifiers(final KeyEvent keyEvent) {
        int modifiers = 0;
        if(keyEvent.isAltPressed()) {
            modifiers |= EkPlatform.KEY_MODIFIER_ALT;
        }
        if(keyEvent.isShiftPressed()) {
            modifiers |= EkPlatform.KEY_MODIFIER_SHIFT;
        }
        if(keyEvent.isCtrlPressed()) {
            modifiers |= EkPlatform.KEY_MODIFIER_CONTROL;
        }
        if(keyEvent.isMetaPressed()) {
            modifiers |= EkPlatform.KEY_MODIFIER_SUPER;
        }
        return modifiers;
    }
}
