package ekapp;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Keep;
import android.util.Log;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import java.util.Locale;

@SuppressLint("Registered")
public class EKActivity extends Activity {

    final static String TAG = "ek:java";

    static {
        System.loadLibrary("native-lib");
    }

    private static EKActivity instance;

    @Keep
    public static Activity getActivity() {
        return instance;
    }

    @Keep
    public static Context getContext() {
        return instance != null ? instance.getApplicationContext() : null;
    }

    @Keep
    public static EKActivity getInstance() {
        return instance;
    }

    public EKSurfaceView glView;
    public RelativeLayout mainLayout;
    private boolean _hasFocus = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        instance = this;

        mainLayout = new RelativeLayout(this);
        mainLayout.setLayoutParams(new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.MATCH_PARENT));
        setContentView(mainLayout);

        EKPlatform.on_startup();
    }

    @Keep
    public static void start_application() {
        EKActivity activity = EKActivity.getInstance();
        EKAudio.register(activity);
        EKPlatform.set_assets_manager(activity.getAssets());
        activity.glView = new EKSurfaceView(activity);
        activity.mainLayout.addView(activity.glView);

        EkExtensionManager.instance.onApplicationStart();
        //RemoveAds.register(activity);
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
        runGLThread(new Runnable() {
            @Override
            public void run() {
                EKPlatform.handle_back_button();
            }
        });
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
        if (Build.VERSION.SDK_INT >= 19) {
            View decorView = getWindow().getDecorView();
            decorView.setSystemUiVisibility(
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                            // Set the content to appear under the system bars so that the
                            // content doesn't resize when the system bars hide and show.
                            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            // Hide the nav bar and status bar
                            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            | View.SYSTEM_UI_FLAG_FULLSCREEN);
        }
    }

    private void resumeIfHasFocus() {
        if (_hasFocus) {
            glView.onResume();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.d(TAG, "Activity onConfigurationChanged");
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "Activity onResume");
        super.onResume();

        EkExtensionManager.instance.onApplicationResume(_hasFocus);
        //Ads.resume();
        resumeIfHasFocus();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        super.onActivityResult(requestCode, resultCode, intent);

        if (GameServices.onActivityResult(requestCode, resultCode, intent)) {
            return;
        }

        //RemoveAds.instance.onActivityResult(requestCode, resultCode, intent);
        EkExtensionManager.instance.onActivityResult(requestCode, resultCode, intent);
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "Activity onPause");
        glView.onPause();
        //Ads.pause();
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
        System.exit(_exitCode);
        super.onDestroy();
    }

    public static void runGLThread(Runnable runnable) {
        instance.glView.queueEvent(runnable);
    }

    public static void runMainThread(Runnable runnable) {
        instance.runOnUiThread(runnable);
    }

    private static int _exitCode = 0;

    @Keep
    public static void app_exit(int code) {
        _exitCode = code;
        // TODO: test quit flow
//        _activity.runOnUiThread(
//                new Runnable() {
//                    @Override
//                    public void run() {
//                        _activity.finish();
//                    }
//                });
        System.exit(_exitCode);
    }

    @Keep
    public static String get_device_lang() {
        return Locale.getDefault().getLanguage();
    }
}
