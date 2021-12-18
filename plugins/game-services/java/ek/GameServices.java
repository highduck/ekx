package ek;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import android.view.View;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInAccount;
import com.google.android.gms.auth.api.signin.GoogleSignInClient;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;
import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.games.AchievementsClient;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.LeaderboardsClient;
import com.google.android.gms.tasks.Task;

@Keep
public class GameServices extends EkPlugin {

    private static final String TAG = "GameServices";
    private static GameServices _instance;
    private final Activity _activity;

    // Request code we use when invoking other Activities to complete the sign-in flow
    final static int RC_RESOLVE = 9001;
    final static int RC_SIGN_IN = 9001;
    final static int RC_LEADERBOARD_UI = 9002;
    final static int RC_ACHIEVEMENTS_UI = 9003;

    private final GoogleSignInOptions _signInOptions;
    private final GoogleSignInClient _googleSignInClient;
    private AchievementsClient _achievements;
    private LeaderboardsClient _leaderboards;
    private final View _layout;
    private boolean _isConnecting = false;
    private boolean _isConnected = false;

    GameServices() {
        Log.d(TAG, "register");
        _activity = EkActivity.getInstance();
        _layout = EkActivity.getInstance().mainLayout;

        _signInOptions = new GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_GAMES_SIGN_IN).build();
        _googleSignInClient = GoogleSignIn.getClient(_activity, _signInOptions);
        silentSignIn();
    }

    @Override
    public void onResume(boolean inFocus) {
        if (_googleSignInClient != null) {
            silentSignIn();
        }
    }

    private void signIn() {
        _activity.startActivityForResult(_googleSignInClient.getSignInIntent(), RC_SIGN_IN);
    }

    private void silentSignIn() {
        if (AppUtils.isTestLab()) {
            return;
        }
        _isConnecting = true;
        _isConnected = false;
        EkActivity.runMainThread(() -> {
            GoogleSignInAccount account = GoogleSignIn.getLastSignedInAccount(_activity);
            if (GoogleSignIn.hasPermissions(account, _signInOptions.getScopeArray())) {
                tryConnectWithAccount(account);
            } else {
                _googleSignInClient.silentSignIn().addOnCompleteListener(_activity, task -> {
                    if (task.isSuccessful()) {
                        Log.d(TAG, "signInSilently(): success");
                        tryConnectWithAccount(task.getResult());
                    } else {
                        Log.d(TAG, "signInSilently(): failure", task.getException());
                        onDisconnected();
                    }
                });
            }
        });
    }

    @Override
    public boolean onActivityResult(int requestCode, int resultCode, Intent intent) {
        if (requestCode == RC_SIGN_IN) {
            Task<GoogleSignInAccount> task = GoogleSignIn.getSignedInAccountFromIntent(intent);
            try {
                GoogleSignInAccount account = task.getResult(ApiException.class);
                tryConnectWithAccount(account);
            } catch (ApiException apiException) {
                String message = "Google Play Services: failed to sign in";
                if (AppUtils.isDebugBuild()) {
                    String err = apiException.getMessage();
                    if (err != null) {
                        message += "\n" + err;
                    }
                }

                onDisconnected();

                AppUtils.alertDebug(message);
            }
            return true;
        }
        return false;
    }

    private void tryConnectWithAccount(@Nullable GoogleSignInAccount account) {
        if (account != null) {
            onConnected(account);
        } else {
            Log.d(TAG, "null account: set state to disconnected");
            onDisconnected();
        }
    }

    private void onConnected(@NonNull GoogleSignInAccount account) {
        Games.getGamesClient(_activity, account).setViewForPopups(_layout);
        _achievements = Games.getAchievementsClient(_activity, account);
        _leaderboards = Games.getLeaderboardsClient(_activity, account);
        _isConnecting = false;
        _isConnected = true;
    }

    private void onDisconnected() {
        _isConnecting = false;
        _isConnected = false;
        _achievements = null;
        _leaderboards = null;
    }

    /// region - methods called from C++

    @Keep
    public static void init() {
        _instance = new GameServices();
        EkPluginManager.instance.extensions.add(_instance);
    }

    @Keep
    public static void leader_board_show(String leaderboard_id) {
        if (_instance._leaderboards != null) {
            Log.d(TAG, "load leaderboard");
            _instance._leaderboards
                    .getLeaderboardIntent(leaderboard_id)
                    .addOnSuccessListener(intent -> {
                        Log.d(TAG, "start leaderboard");
                        _instance._activity.startActivityForResult(intent, RC_LEADERBOARD_UI);
                    });
        } else {
            Log.d(TAG, "[leaderboard_show] not ready");
            if (!_instance._isConnecting && !_instance._isConnected) {
                _instance.signIn();
            }
        }
    }

    @Keep
    public static void leader_board_submit(final String leaderboard_id, final int score) {
        if (_instance._leaderboards != null) {
            Log.d(TAG, "submit leaderboard: " + leaderboard_id + " " + score);
            _instance._leaderboards.submitScore(leaderboard_id, score);
        } else {
            Log.d(TAG, "[leaderboard_submit] not ready");
        }
    }

    @Keep
    public static void achievement_update(final String achievement_id, final int increment) {
        if (_instance._achievements != null) {
            if (increment > 0) {
                Log.d(TAG, "increment achievement");
                _instance._achievements.increment(achievement_id, increment);
            } else {
                Log.d(TAG, "unlock achievement");
                _instance._achievements.unlock(achievement_id);
            }
        } else {
            Log.d(TAG, "[achievement_update] not ready");
        }
    }

    @Keep
    public static void achievement_show() {
        if (_instance._achievements != null) {
            Log.d(TAG, "load achievements");
            _instance._achievements.getAchievementsIntent().addOnSuccessListener(intent -> {
                Log.d(TAG, "show achievements");
                _instance._activity.startActivityForResult(intent, RC_ACHIEVEMENTS_UI);
            });
        } else {
            Log.d(TAG, "[achievement_show] not ready");
            if (!_instance._isConnecting && !_instance._isConnected) {
                _instance.signIn();
            }
        }
    }
}
