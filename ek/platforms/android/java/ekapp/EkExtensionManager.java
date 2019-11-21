package ekapp;

import android.content.Intent;

import java.util.ArrayList;

public final class EkExtensionManager implements EkExtension {

    final public static EkExtensionManager instance = new EkExtensionManager();

    final public ArrayList<EkExtension> extensions = new ArrayList<>();

    public void onApplicationStart() {
        for (EkExtension extension : extensions) {
            extension.onApplicationStart();
        }
    }

    public void onApplicationResume(boolean inFocus) {
        for (EkExtension extension : extensions) {
            extension.onApplicationResume(inFocus);
        }
    }

    public void onActivityResult(int requestCode, int resultCode, Intent intent) {
        for (EkExtension extension : extensions) {
            extension.onActivityResult(requestCode, resultCode, intent);
        }
    }

    public void onApplicationPause() {
        for (EkExtension extension : extensions) {
            extension.onApplicationPause();
        }
    }
}
