package ek;

import android.content.Intent;
import android.graphics.Rect;
import android.net.Uri;
import android.os.Build;
import android.view.DisplayCutout;
import android.view.Window;

import androidx.annotation.Keep;

import java.util.List;
import java.util.Locale;

@Keep
public final class EkDevice {

    @Keep
    public static int openURL(final String url) {
        final EkActivity activity = EkActivity.getInstance();
        activity.runOnUiThread(() -> {
            try {
                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
                int flags = Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_ACTIVITY_MULTIPLE_TASK;
                if (Build.VERSION.SDK_INT >= 21) {
                    flags |= Intent.FLAG_ACTIVITY_NEW_DOCUMENT;
                } else {
                    flags |= Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET;
                }
                intent.addFlags(flags);
                activity.startActivity(intent);
            } catch (Exception ignored) {
            }
        });
        return 0;
    }

    @Keep
    public static String getLanguage() {
        return Locale.getDefault().getLanguage();
    }

    @Keep
    public static int share(final String text) {
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
        return 0;
    }
}
