package ekapp;

import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class EKRenderer implements GLSurfaceView.Renderer {

    public int width = 1;
    public int height = 1;
    public float scaleFactor = 1.0f;
    public EKSurfaceView view;

    public EKRenderer(EKSurfaceView view) {
        this.view = view;
    }

    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        width = view.getWidth();
        height = view.getHeight();
        Log.i("ekapp", "onSurfaceCreated: " + width + " x " + height);
        EKPlatform.handle_resize(width, height, scaleFactor);

        GLES20.glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        EKPlatform.on_ready();
    }

    public void onSurfaceChanged(GL10 unused, int width, int height) {
        Log.i("ekapp", "onSurfaceChanged: " + width + " x " + height);
        this.width = width;
        this.height = height;
        EKPlatform.handle_resize(width, height, scaleFactor);
    }

    public void onDrawFrame(GL10 unused) {
        try {
            EKPlatform.handle_enter_frame();
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