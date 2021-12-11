package ek;

import android.content.Intent;

public interface EkPluginInterface {

    void onStart();

    void onResume(boolean inFocus);

    boolean onActivityResult(int requestCode, int resultCode, Intent intent);

    void onPause();

    void onDestroy();
}
