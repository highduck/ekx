package ek;

import android.graphics.Rect;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.util.Log;
import android.view.DisplayCutout;
import android.view.Window;

import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class EkRenderer implements GLSurfaceView.Renderer {

    public int width = 1;
    public int height = 1;
    public float scaleFactor = 1.0f;
    public EkSurfaceView view;

    public EkRenderer(EkSurfaceView view) {
        this.view = view;
    }

    private void fireResize() {
        final float[] values = new float[]{
                this.width,
                this.height,
                this.scaleFactor,
                0.0f,
                0.0f,
                0.0f,
                0.0f
        };
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            Window cocosWindow = EkActivity.getInstance().getWindow();
            DisplayCutout displayCutout = cocosWindow.getDecorView().getRootWindowInsets().getDisplayCutout();
            // Judge whether it is cutouts (aka notch) screen phone by judge cutout equle to null
            if (displayCutout != null) {
                List<Rect> rects = displayCutout.getBoundingRects();
                // Judge whether it is cutouts (aka notch) screen phone by judge cutout rects is null or zero size
                if (rects.size() != 0) {
                    values[3] = displayCutout.getSafeInsetLeft();
                    values[4] = displayCutout.getSafeInsetTop();
                    values[5] = displayCutout.getSafeInsetRight();
                    values[6] = displayCutout.getSafeInsetBottom();
                }
            }
        }
        EkPlatform.sendResize(values);
    }

    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        width = view.getWidth();
        height = view.getHeight();
        scaleFactor = view.getContext().getResources().getDisplayMetrics().density;

        Log.i("ek", "onSurfaceCreated: " + width + " x " + height);
        fireResize();

        GLES20.glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        EkPlatform.notifyReady();
    }

    public void onSurfaceChanged(GL10 unused, int width, int height) {
        Log.i("ek", "onSurfaceChanged: " + width + " x " + height);
        this.width = width;
        this.height = height;
        fireResize();
    }

    public void onDrawFrame(GL10 unused) {
        try {
            EkPlatform.processFrame();

            // since it solves v-sync stalling (dequeBuffer issue)
            GLES20.glFinish();
        } catch (Exception e) {
            GLES20.glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        }
    }

    public void onResume() {
    }

    public void onPause() {
    }
}