package ekapp;

import android.app.Activity;
import android.app.AlertDialog;
import android.provider.Settings;

public class AppUtils {

    private static boolean DEBUG_BUILD = false;

    static public void setDebugBuild(boolean debugBuild) {
        DEBUG_BUILD = debugBuild;
    }

    static public boolean isDebugBuild() {
        return DEBUG_BUILD;
    }

    static public boolean isTestLab() {
        final Activity activity = EKActivity.getActivity();
        String testLabSetting = Settings.System.getString(activity.getContentResolver(), "firebase.test.lab");
        return "true".equals(testLabSetting);
    }

    static public void alertDebug(String message) {
        if (!isDebugBuild()) {
            return;
        }
        final Activity activity = EKActivity.getActivity();
        new AlertDialog.Builder(activity)
                .setMessage(message)
                .setNeutralButton(android.R.string.ok, null)
                .show();
    }
}
