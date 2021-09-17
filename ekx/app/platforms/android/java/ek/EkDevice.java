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
    public static int[] getScreenInsets() {
        final int[] safeInsets = new int[]{0, 0, 0, 0};
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            Window cocosWindow = EkActivity.getInstance().getWindow();
            DisplayCutout displayCutout = cocosWindow.getDecorView().getRootWindowInsets().getDisplayCutout();
            // Judge whether it is cutouts (aka notch) screen phone by judge cutout equle to null
            if (displayCutout != null) {
                List<Rect> rects = displayCutout.getBoundingRects();
                // Judge whether it is cutouts (aka notch) screen phone by judge cutout rects is null or zero size
                if (rects.size() != 0) {
                    safeInsets[0] = displayCutout.getSafeInsetLeft();
                    safeInsets[1] = displayCutout.getSafeInsetTop();
                    safeInsets[2] = displayCutout.getSafeInsetRight();
                    safeInsets[3] = displayCutout.getSafeInsetBottom();
                }
            }
        }
        return safeInsets;
    }
}
