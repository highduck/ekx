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
            final int id = event.getPointerId(i) + 1;
            final float x = event.getX(i);
            final float y = event.getY(i);

            if (act == MotionEvent.ACTION_DOWN || act == MotionEvent.ACTION_POINTER_DOWN) {
                handleTouchEvent(EkPlatform.TOUCH_START, id, x, y);
                return true;
            } else if (act == MotionEvent.ACTION_MOVE) {
                for (int j = 0; j < event.getPointerCount(); ++j) {
                    final int touchId = event.getPointerId(j) + 1;
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

    @Override
    public boolean onKeyDown(final int keyCode, final KeyEvent keyEvent) {
        final int _keyCode = EkPlatform._KeyCode(keyCode);
        if(_keyCode > 0) {
            final int modifiers = EkPlatform._KeyModifiers(keyEvent);
            handleKeyEvent(EkPlatform.KEY_DOWN, _keyCode, modifiers);
            return true;
        }
        return super.onKeyDown(keyCode, keyEvent);
    }

    @Override
    public boolean onKeyUp(final int keyCode, final KeyEvent keyEvent) {
        final int _keyCode = EkPlatform._KeyCode(keyCode);
        if(_keyCode > 0) {
            final int modifiers = EkPlatform._KeyModifiers(keyEvent);
            handleKeyEvent(EkPlatform.KEY_UP, _keyCode, modifiers);
            return true;
        }
        return super.onKeyDown(keyCode, keyEvent);
    }

    private void handleKeyEvent(final int type, final int code, final int modifiers) {
        queueEvent(() -> EkPlatform.sendKeyEvent(type, code, modifiers));
    }
}