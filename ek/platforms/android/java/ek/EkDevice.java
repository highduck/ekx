package ek;

import android.content.Context;
import android.graphics.Rect;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.view.DisplayCutout;
import android.view.Window;

import androidx.annotation.Keep;

import java.util.List;
import java.util.Locale;

public final class EkDevice {

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

    private static Vibrator _vibrator;

    @Keep
    public static void vibrate(long durationMillis) {
        if (_vibrator == null) {
            _vibrator = (Vibrator) EkActivity.getActivity().getSystemService(Context.VIBRATOR_SERVICE);
        }

        if (_vibrator != null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                _vibrator.vibrate(VibrationEffect.createOneShot(durationMillis, VibrationEffect.DEFAULT_AMPLITUDE));
            } else {
                _vibrator.vibrate(durationMillis);
            }
        }
    }
}
