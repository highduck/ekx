package ek;

import static android.app.Activity.RESULT_OK;

import android.content.Intent;
import android.os.Bundle;

import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import com.firebase.ui.auth.AuthUI;
import com.firebase.ui.auth.FirebaseAuthUIActivityResultContract;
import com.firebase.ui.auth.IdpResponse;
import com.firebase.ui.auth.data.model.FirebaseAuthUIAuthenticationResult;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.analytics.FirebaseAnalytics;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;

import java.util.Arrays;
import java.util.List;

@Keep
public class FirebasePlugin {

    final private static String TAG = "FirebasePlugin";

    private static FirebaseAnalytics _analytics;

    @Keep
    public static void call(int method) {
        if (method == 0) {
            // init
            _analytics = FirebaseAnalytics.getInstance(EkActivity.getInstance());

            signInLauncher = EkActivity.getInstance().registerForActivityResult(
                    new FirebaseAuthUIActivityResultContract(),
                    new ActivityResultCallback<FirebaseAuthUIAuthenticationResult>() {
                        @Override
                        public void onActivityResult(FirebaseAuthUIAuthenticationResult result) {
                            onSignInResult(result);
                        }
                    }
            );
        } else if (method == 1) {
            createSignInIntent();
        } else if (method == 2) {
            signOut();
        } else if (method == 3) {
            delete();
        }
        // Test Crash:
        // throw new RuntimeException("Test Crash"); // Force a crash
        // then restart application
        // check console
    }

    @Keep
    public static void set_screen(final String s) {
        // TODO: deprecated, to log screen we need to include current screen as param to events?
        // https://firebase.googleblog.com/2020/08/google-analytics-manual-screen-view.html

        //_activity.runOnUiThread(() -> _analytics.setCurrentScreen(_activity, s, null));
    }

    @Keep
    public static void send_event(final String action, final String item) {
        EkActivity.runMainThread(() -> {
            final Bundle bundle = new Bundle();
            bundle.putString(FirebaseAnalytics.Param.ITEM_NAME, item);
            _analytics.logEvent(action, bundle);
        });
    }

    // log internal game events from UI thread
    public static void logEvent(String eventType, Bundle params) {
        if (_analytics != null) {
            _analytics.logEvent(eventType, params);
        }
    }

    /**
     * auth
     **/
    // See: https://developer.android.com/training/basics/intents/result
    private static ActivityResultLauncher<Intent> signInLauncher;

    public static void createSignInIntent() {
        // [START auth_fui_create_intent]
        // Choose authentication providers
        List<AuthUI.IdpConfig> providers = Arrays.asList(
//                new AuthUI.IdpConfig.PhoneBuilder().build(),
//                new AuthUI.IdpConfig.GoogleBuilder().build(),
//                new AuthUI.IdpConfig.FacebookBuilder().build(),
//                new AuthUI.IdpConfig.TwitterBuilder().build(),
                new AuthUI.IdpConfig.EmailBuilder().build(),
                new AuthUI.IdpConfig.AnonymousBuilder().build()
        );

        // Create and launch sign-in intent
        Intent signInIntent = AuthUI.getInstance()
                .createSignInIntentBuilder()
                .setAvailableProviders(providers)
                .build();
        signInLauncher.launch(signInIntent);
        // [END auth_fui_create_intent]
    }

    // [START auth_fui_result]
    private static void onSignInResult(FirebaseAuthUIAuthenticationResult result) {
        IdpResponse response = result.getIdpResponse();
        if (result.getResultCode() == RESULT_OK) {
            // Successfully signed in
            FirebaseUser user = FirebaseAuth.getInstance().getCurrentUser();
            // ...
        } else {
            // Sign in failed. If response is null the user canceled the
            // sign-in flow using the back button. Otherwise check
            // response.getError().getErrorCode() and handle the error.
            // ...
        }
    }
    // [END auth_fui_result]

    public static void signOut() {
        // [START auth_fui_signout]
        AuthUI.getInstance()
                .signOut(EkActivity.getInstance())
                .addOnCompleteListener(new OnCompleteListener<Void>() {
                    public void onComplete(@NonNull Task<Void> task) {
                        // ...
                    }
                });
        // [END auth_fui_signout]
    }

    public static void delete() {
        // [START auth_fui_delete]
        AuthUI.getInstance()
                .delete(EkActivity.getInstance())
                .addOnCompleteListener(new OnCompleteListener<Void>() {
                    @Override
                    public void onComplete(@NonNull Task<Void> task) {
                        // ...
                    }
                });
        // [END auth_fui_delete]
    }
}
