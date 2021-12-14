package ek;

import android.os.Bundle;

import androidx.annotation.Keep;

import com.google.firebase.analytics.FirebaseAnalytics;

@Keep
public class FirebasePlugin {

    final private static String TAG = "FirebasePlugin";

    private static FirebaseAnalytics _analytics;

    @Keep
    public static void init() {
        _analytics = FirebaseAnalytics.getInstance(EkActivity.getInstance());

        // Test Crash:
        // throw new RuntimeException("Test Crash"); // Force a crash
        // then restart application
        // check console
    }

    @Keep
    public static void set_screen(final String s) {
        // TODO: deprecated, to log screen we need to include current screen as param to events?
        // https://firebase.googleblog.com/2020/08/google-analytics-manual-screen-view.html

        //_activity.runOnUiThread(() -> _analytics.setCurrentScreen(_activity, s, null));
    }

    @Keep
    public static void send_event(final String action, final String item) {
        EkActivity.runMainThread(() -> {
            final Bundle bundle = new Bundle();
            bundle.putString(FirebaseAnalytics.Param.ITEM_NAME, item);
            _analytics.logEvent(action, bundle);
        });
    }

    // log internal game events from UI thread
    public static void logEvent(String eventType, Bundle params) {
        if (_analytics != null) {
            _analytics.logEvent(eventType, params);
        }
    }
}
