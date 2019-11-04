package ekapp;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.Keep;
import android.util.Log;

import com.google.firebase.analytics.FirebaseAnalytics;

@Keep
public class Analytics {

    final private static String TAG = "Analytics";

    private static Activity _activity;
    private static FirebaseAnalytics _analytics;

    @Keep
    public static void init() {
        _activity = EKActivity.getInstance();
        _analytics = FirebaseAnalytics.getInstance(EKActivity.getInstance());
        _analytics.setAnalyticsCollectionEnabled(true);
        //_analytics.setMinimumSessionDuration(1000 * 4);
        _analytics.setSessionTimeoutDuration(1000 * 60 * 30);
    }

    @Keep
    public static void set_screen(final String s) {
        _activity.runOnUiThread(
                new Runnable() {
                    @Override
                    public void run() {
                        _analytics.setCurrentScreen(_activity, s, null);
                    }
                });

    }

    @Keep
    public static void send_event(final String action, final String item) {
        _activity.runOnUiThread(
                new Runnable() {
                    @Override
                    public void run() {
                        final Bundle bundle = new Bundle();
                        bundle.putString(FirebaseAnalytics.Param.ITEM_NAME, item);
                        _analytics.logEvent(action, bundle);
                    }
                });
    }

    // log internal game events from UI thread
    public static void logEvent(String eventType, Bundle params) {
        if (_analytics != null) {
            _analytics.logEvent(eventType, params);
        }
    }
}
