package ekapp;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.annotation.Keep;

import com.google.firebase.analytics.FirebaseAnalytics;

public class Sharing {

    @Keep
    public static void navigate(final String url) {
        final EKActivity activity = EKActivity.getInstance();
        activity.runOnUiThread(
                new Runnable() {
                    @Override
                    public void run() {
                        try {
                            Intent rateIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                            activity.startActivity(rateIntent);
                        } catch (Exception ignored) {
                        }
                    }
                });
    }

    @Keep
    public static void rate_us(final String appId) {
        final EKActivity activity = EKActivity.getInstance();
        activity.runOnUiThread(
                new Runnable() {
                    @Override
                    public void run() {
                        try {
                            final Bundle bundle = new Bundle();
                            bundle.putString(FirebaseAnalytics.Param.ITEM_ID, "market");
                            Analytics.logEvent("view_item", bundle);

                            Intent rateIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=" + appId));
                            activity.startActivity(rateIntent);
                        } catch (Exception ignored) {
                        }
                    }
                });
    }

    @Keep
    public static void send_message(final String text) {
        final EKActivity activity = EKActivity.getInstance();
        activity.runOnUiThread(
                new Runnable() {
                    @Override
                    public void run() {
                        try {

                            final Bundle bundle = new Bundle();
                            bundle.putString(FirebaseAnalytics.Param.CONTENT_TYPE, "text");
                            Analytics.logEvent("share", bundle);

                            Intent intent = new Intent();
                            intent.setAction(Intent.ACTION_SEND);
                            intent.setType("text/plain");
                            intent.putExtra(Intent.EXTRA_TEXT, text);
                            activity.startActivity(Intent.createChooser(intent, "Share"));
                        } catch (Exception ignored) {
                        }
                    }
                });
    }
}
