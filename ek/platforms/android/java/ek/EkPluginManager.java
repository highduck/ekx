package ek;

import android.content.Intent;

import java.util.ArrayList;

public final class EkPluginManager implements EkPluginInterface {

    final public static EkPluginManager instance = new EkPluginManager();

    final public ArrayList<EkPluginInterface> extensions = new ArrayList<>();

    public void onStart() {
        for (EkPluginInterface extension : extensions) {
            extension.onStart();
        }
    }

    public void onResume(boolean inFocus) {
        for (EkPluginInterface extension : extensions) {
            extension.onResume(inFocus);
        }
    }

    public void onActivityResult(int requestCode, int resultCode, Intent intent) {
        for (EkPluginInterface extension : extensions) {
            extension.onActivityResult(requestCode, resultCode, intent);
        }
    }

    public void onPause() {
        for (EkPluginInterface extension : extensions) {
            extension.onPause();
        }
    }

    public void onDestroy() {
        for (EkPluginInterface extension : extensions) {
            extension.onDestroy();
        }
    }
}
