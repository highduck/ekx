package ek;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.DisplayCutout;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import java.util.List;
import java.util.Locale;

@SuppressLint("Registered")
public class EkActivity extends Activity {

    final static String TAG = "ek";

    private static EkActivity instance;

    @Keep
    public static Activity getActivity() {
        return instance;
    }

    @Keep
    public static Context getContext() {
        return instance != null ? instance.getApplicationContext() : null;
    }

    @Keep
    public static EkActivity getInstance() {
        return instance;
    }

    public EkSurfaceView glView;
    public RelativeLayout mainLayout;
    private boolean _hasFocus = false;

    protected void onLoadNativeLibraries() {
        try {
            System.loadLibrary("native-lib");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        instance = this;

        hideSystemUI();

        onLoadNativeLibraries();

        mainLayout = new RelativeLayout(this);
        mainLayout.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT));
        setContentView(mainLayout);

        EkPlatform.sendEvent(EkPlatform.CALL_START);
    }

    @Keep
    public static void start_application() {
        EkActivity activity = EkActivity.getInstance();
        EkPlatform.initAssets(activity.getAssets());
        activity.glView = new EkSurfaceView(activity);
        activity.mainLayout.addView(activity.glView);

        EkExtensionManager.instance.onApplicationStart();
    }

    @Override
    public void onStart() {
        super.onStart();
        GameServices.onResume();
    }

//    @Override
//    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
//        if (requestCode == 101 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
//            // This method is called when the  permissions are given
//        }
//    }

    @Override
    public void onBackPressed() {
        runGLThread(() -> EkPlatform.sendEvent(EkPlatform.BACK_BUTTON));
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        Log.d(TAG, "onWindowFocusChanged: " + hasFocus);
        super.onWindowFocusChanged(hasFocus);

        _hasFocus = hasFocus;
        if (hasFocus) {
            hideSystemUI();
        }
        resumeIfHasFocus();
    }

    protected void hideSystemUI() {
        final Window window = getWindow();

        // Set the content to appear under the system bars so that the
        // content doesn't resize when the system bars hide and show.
        int flags = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                // Hide the nav bar and status bar
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN;
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            flags |= View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        }
        View decorView = window.getDecorView();
        decorView.setSystemUiVisibility(flags);

        final WindowManager.LayoutParams lp = window.getAttributes();
        lp.flags = lp.flags | WindowManager.LayoutParams.FLAG_FULLSCREEN;
        // Make sure we're running on Pie or higher to change cutout mode
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            // Enable rendering into the cutout area
            lp.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }

        window.setAttributes(lp);
    }

    private void resumeIfHasFocus() {
        if (_hasFocus) {
            glView.onResume();
        }
    }

    @Override
    public void onConfigurationChanged(@NonNull Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.d(TAG, "Activity onConfigurationChanged");
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "Activity onResume");
        super.onResume();

        EkExtensionManager.instance.onApplicationResume(_hasFocus);
        resumeIfHasFocus();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);

        if (GameServices.onActivityResult(requestCode, resultCode, intent)) {
            return;
        }

        EkExtensionManager.instance.onActivityResult(requestCode, resultCode, intent);
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "Activity onPause");
        glView.onPause();
        EkExtensionManager.instance.onApplicationPause();
        super.onPause();
    }

    @Override
    protected void onRestart() {
        Log.d(TAG, "Activity onRestart");
        super.onRestart();
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "Activity onStop");
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
//        System.exit(0);
    }

    public static void runGLThread(Runnable runnable) {
        instance.glView.queueEvent(runnable);
    }

    public static void runMainThread(Runnable runnable) {
        instance.runOnUiThread(runnable);
    }

    @Keep
    public static void app_exit(int code) {
        instance.runOnUiThread(() -> {
            if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                instance.finishAndRemoveTask();
            }
            //instance.finishAffinity();
            System.exit(0);
        });
    }

    @Keep
    public static String get_device_lang() {
        return Locale.getDefault().getLanguage();
    }

    @Keep
    public static int[] getSafeInsets() {
        final int[] safeInsets = new int[]{0, 0, 0, 0};
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            Window cocosWindow = instance.getWindow();
            DisplayCutout displayCutout = cocosWindow.getDecorView().getRootWindowInsets().getDisplayCutout();
            // Judge whether it is cutouts (aka notch) screen phone by judge cutout equle to null
            if (displayCutout != null) {
                List<Rect> rects = displayCutout.getBoundingRects();
                // Judge whether it is cutouts (aka notch) screen phone by judge cutout rects is null or zero size
                if (rects.size() != 0) {
                    safeInsets[0] = displayCutout.getSafeInsetBottom();
                    safeInsets[1] = displayCutout.getSafeInsetLeft();
                    safeInsets[2] = displayCutout.getSafeInsetRight();
                    safeInsets[3] = displayCutout.getSafeInsetTop();
                }
            }
        }
        return safeInsets;
    }
}
