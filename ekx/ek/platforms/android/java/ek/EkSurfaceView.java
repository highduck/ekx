package ek;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

import java.util.HashMap;

public class EkSurfaceView extends GLSurfaceView {

    final static String TAG = "ek";

    public final EkRenderer renderer;

    public EkSurfaceView(Context context, boolean needDepth) {
        super(context);

        setEGLContextClientVersion(2);
        setEGLConfigChooser(needDepth);
        setPreserveEGLContextOnPause(true);

        renderer = new EkRenderer(this);
        setRenderer(renderer);

        setFocusableInTouchMode(true);
        setFocusable(true);
    }

    @Override
    public void onResume() {
        Log.i(TAG, "GLSurface onResume");
        super.onResume();

        this.setRenderMode(RENDERMODE_CONTINUOUSLY);
        this.queueEvent(() -> {
            EkPlatform.sendEvent(EkPlatform.RESUME);
            EkSurfaceView.this.renderer.onResume();
        });
    }

    @Override
    public void onPause() {
        Log.i(TAG, "GLSurface onPause");
        this.queueEvent(() -> {
            EkPlatform.sendEvent(EkPlatform.PAUSE);
            EkSurfaceView.this.renderer.onPause();
        });
        this.setRenderMode(RENDERMODE_WHEN_DIRTY);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event != null) {

            final int i = event.getActionIndex();
            final int act = event.getActionMasked();
            final int id = event.getPointerId(i);
            final float x = event.getX(i);
            final float y = event.getY(i);

            if (act == MotionEvent.ACTION_DOWN || act == MotionEvent.ACTION_POINTER_DOWN) {
                handleTouchEvent(EkPlatform.TOUCH_BEGIN, id, x, y);
                return true;
            } else if (act == MotionEvent.ACTION_MOVE) {
                for (int j = 0; j < event.getPointerCount(); ++j) {
                    final int touchId = event.getPointerId(j);
                    final float touchX = event.getX(j);
                    final float touchY = event.getY(j);
                    handleTouchEvent(EkPlatform.TOUCH_MOVE, touchId, touchX, touchY);
                }
                return true;
            } else if (act == MotionEvent.ACTION_UP || act == MotionEvent.ACTION_POINTER_UP || act == MotionEvent.ACTION_CANCEL) {
                handleTouchEvent(EkPlatform.TOUCH_END, id, x, y);
                return true;
            }

            return true;
        }
        return false;
    }

    private void handleTouchEvent(final int type, final int id, final float x, final float y) {
        queueEvent(() -> EkPlatform.sendTouch(type, id, x, y));
    }

    private static int convertKeyCode(final int keyCode) {
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

    @Override
    public boolean onKeyDown(final int keyCode, final KeyEvent keyEvent) {
        int code = convertKeyCode(keyCode);
        if(code > 0) {
            handleKeyEvent(EkPlatform.KEY_DOWN, code, 0);
            return true;
        }
        return super.onKeyDown(keyCode, keyEvent);
    }

    @Override
    public boolean onKeyUp(final int keyCode, final KeyEvent keyEvent) {
        int code = convertKeyCode(keyCode);
        if(code > 0) {
            handleKeyEvent(EkPlatform.KEY_UP, code, 0);
            return true;
        }
        return super.onKeyDown(keyCode, keyEvent);
    }

    private void handleKeyEvent(final int type, final int code, final int modifiers) {
        queueEvent(() -> EkPlatform.sendKeyEvent(type, code, modifiers));
    }
}