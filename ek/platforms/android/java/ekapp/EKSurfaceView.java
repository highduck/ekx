package ekapp;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.MotionEvent;

public class EKSurfaceView extends GLSurfaceView {

    public final EKRenderer renderer;

    public EKSurfaceView(Context context) {
        super(context);

        setEGLContextClientVersion(2);
        setEGLConfigChooser(false);
        setPreserveEGLContextOnPause(true);

        renderer = new EKRenderer(this);
        setRenderer(renderer);
    }

    @Override
    public void onResume() {
        Log.i("ek:java", "GLSurface onResume");
        super.onResume();

        this.setRenderMode(RENDERMODE_CONTINUOUSLY);
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                EKPlatform.handle_resume();
                EKSurfaceView.this.renderer.onResume();
            }
        });
        EKAudio.onResume();
    }

    @Override
    public void onPause() {
        Log.i("ek:java", "GLSurface onPause");
        EKAudio.onPause();
        this.queueEvent(new Runnable() {
            @Override
            public void run() {
                EKPlatform.handle_pause();
                EKSurfaceView.this.renderer.onPause();
            }
        });
        this.setRenderMode(RENDERMODE_WHEN_DIRTY);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event != null) {

            final int TOUCH_BEGIN = 1;
            final int TOUCH_MOVE = 2;
            final int TOUCH_END = 3;

            final int i = event.getActionIndex();
            final int act = event.getActionMasked();
            final int id = event.getPointerId(i);
            final float x = event.getX(i);
            final float y = event.getY(i);

            if (act == MotionEvent.ACTION_DOWN || act == MotionEvent.ACTION_POINTER_DOWN) {
                handleTouchEvent(TOUCH_BEGIN, id, x, y);
                return true;
            } else if (act == MotionEvent.ACTION_MOVE) {
                for (int j = 0; j < event.getPointerCount(); ++j) {
                    final int touchId = event.getPointerId(j);
                    final float touchX = event.getX(j);
                    final float touchY = event.getY(j);
                    handleTouchEvent(TOUCH_MOVE, touchId, touchX, touchY);
                }
                return true;
            } else if (act == MotionEvent.ACTION_UP || act == MotionEvent.ACTION_POINTER_UP || act == MotionEvent.ACTION_CANCEL) {
                handleTouchEvent(TOUCH_END, id, x, y);
                return true;
            }

            return true;
        }
        return false;
    }

    private void handleTouchEvent(final int type, final int id, final float x, final float y) {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                EKPlatform.handle_touch_event(type, id, x, y);
            }
        });
    }
}