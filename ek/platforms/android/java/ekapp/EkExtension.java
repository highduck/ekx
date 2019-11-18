package ekapp;

import android.content.Intent;

public interface EkExtension {

    void onApplicationStart();

    void onApplicationResume(boolean inFocus);

    void onActivityResult(int requestCode, int resultCode, Intent intent);

    void onApplicationPause();
}
