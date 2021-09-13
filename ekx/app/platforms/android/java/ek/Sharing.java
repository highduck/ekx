package ek;

import android.content.Intent;
import android.net.Uri;

import androidx.annotation.Keep;

@Keep
public class Sharing {

    @Keep
    public static void rate_us(final String appId) {
        final EkActivity activity = EkActivity.getInstance();
        activity.runOnUiThread(() -> {
            try {
                Intent rateIntent = new Intent(Intent.ACTION_VIEW,
                        Uri.parse("market://details?id=" + appId));
                activity.startActivity(rateIntent);
            } catch (Exception ignored) {
            }
        });
    }

    @Keep
    public static void send_message(final String text) {
        final EkActivity activity = EkActivity.getInstance();
        activity.runOnUiThread(() -> {
            try {
                Intent intent = new Intent();
                intent.setAction(Intent.ACTION_SEND);
                intent.setType("text/plain");
                intent.putExtra(Intent.EXTRA_TEXT, text);
                activity.startActivity(Intent.createChooser(intent, "Share"));
            } catch (Exception ignored) {
            }
        });
    }
}
