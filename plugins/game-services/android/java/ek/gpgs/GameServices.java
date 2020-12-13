package ek.gpgs;

import ek.EkActivity;
import ek.AppUtils;
import ek.Analytics;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import androidx.annotation.Keep;

import com.google.android.gms.auth.api.signin.GoogleSignIn;
import com.google.android.gms.auth.api.signin.GoogleSignInAccount;
import com.google.android.gms.auth.api.signin.GoogleSignInClient;
import com.google.android.gms.auth.api.signin.GoogleSignInOptions;
import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.games.AchievementsClient;
import com.google.android.gms.games.Games;
import com.google.android.gms.games.LeaderboardsClient;
import com.google.android.gms.tasks.Task;
import com.google.firebase.analytics.FirebaseAnalytics;

@Keep
public class GameServices {

    private static final String TAG = "GameServices";
    //    private static GoogleApiClient _googleApiClient;
    private static Activity _activity;

    //    private static GPSGamesListener _listener;
    // Request code we use when invoking other Activities to complete the
    // sign-in flow.
    final static int RC_RESOLVE = 9001;
    final static int RC_SIGN_IN = 9001;
    final static int RC_LEADERBOARD_UI = 9002;
    final static int RC_ACHIEVEMENTS_UI = 9003;


    private static GoogleSignInClient _googleSignInClient;
    private static AchievementsClient _achievements;
    private static LeaderboardsClient _leaderboards;
    private static View _layout;
    private static boolean _isConnecting = false;
    private static boolean _isConnected = false;
//    private static boolean _silentSignInFailed = false;

    @Keep
    public static void init() {
        Log.d(TAG, "register");
        _activity = EkActivity.getInstance();
        _layout = EkActivity.getInstance().mainLayout;
        _googleSignInClient = GoogleSignIn.getClient(_activity,
                new GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_GAMES_SIGN_IN)
                        // OAuth 2.0 API client id
//                        .requestIdToken(_activity.getString(R.string.server_client_id))
                        .build());
        silentSignIn();
    }

    public static void onResume() {
        if (_googleSignInClient != null) {
            silentSignIn();
        }
    }

    private static void signIn() {
        _activity.startActivityForResult(_googleSignInClient.getSignInIntent(), RC_SIGN_IN);
    }

    private static void silentSignIn() {
        if (AppUtils.isTestLab()) {
            return;
        }
        _isConnecting = true;
        _isConnected = false;
        _googleSignInClient.silentSignIn().addOnCompleteListener(_activity,
                task -> {
                    if (task.isSuccessful()) {
                        Log.d(TAG, "signInSilently(): success");
                        onConnected(task.getResult());
                    } else {
                        Log.d(TAG, "signInSilently(): failure", task.getException());
                        onDisconnected();
//                            _silentSignInFailed = true;
                    }
                });
    }

    @Keep
    public static void leader_board_show(String leaderboard_id) {
        if (_leaderboards != null) {
            Log.d(TAG, "load leaderboard");
            _leaderboards
                    .getLeaderboardIntent(leaderboard_id)
                    .addOnSuccessListener(intent -> {
                        Log.d(TAG, "start leaderboard");
                        _activity.startActivityForResult(intent, RC_LEADERBOARD_UI);
                    });
        } else {
            Log.d(TAG, "[leaderboard_show] not ready");
            if (!_isConnecting && !_isConnected) {
                signIn();
            }
        }
    }

    @Keep
    public static void leader_board_submit(final String leaderboard_id, final int score) {
        EkActivity.runMainThread(
                () -> {
                    final Bundle bundle = new Bundle();
                    bundle.putString(FirebaseAnalytics.Param.SCORE, String.valueOf(score));
                    Analytics.logEvent("post_score", bundle);

                    if (_leaderboards != null) {
                        Log.d(TAG, "submit leaderboard: " + leaderboard_id + " " + score);
                        // https://stackoverflow.com/questions/39284039/google-play-games-leaderboard-closes-automatically?rq=1
                        //_leaderboards.submitScore(leaderboard_id, score);

                        _leaderboards.submitScoreImmediate(leaderboard_id, score).addOnCompleteListener(t -> {
                            if (t.isSuccessful()) {
                                Log.d(TAG, "leaderboard_submit successful!");
                            } else {
                                if (t.getException() != null) {
                                    Log.d(TAG, "leaderboard_submit error: " + t.getException().getMessage());
                                } else {
                                    Log.d(TAG, "leaderboard_submit unknown error");
                                }
                            }
                        });
                    } else {
                        Log.d(TAG, "[leaderboard_submit] not ready");
                    }
                }
        );
    }

    @Keep
    public static void achievement_update(final String achievement_id, final int increment) {
        if (_achievements != null) {
            if (increment > 0) {
                Log.d(TAG, "increment achievement");
                _achievements.increment(achievement_id, increment);
            } else {
                Log.d(TAG, "unlock achievement");
                _achievements.unlock(achievement_id);
            }
        } else {
            Log.d(TAG, "[achievement_update] not ready");
        }
    }

    @Keep
    public static void achievement_show() {
        if (_achievements != null) {
            Log.d(TAG, "load achievements");
            _achievements.getAchievementsIntent().addOnSuccessListener(intent -> {
                Log.d(TAG, "show achievements");
                _activity.startActivityForResult(intent, RC_ACHIEVEMENTS_UI);
            });
        } else {
            Log.d(TAG, "[achievement_show] not ready");
            if (!_isConnecting && !_isConnected) {
                signIn();
            }
        }
    }

    public static boolean onActivityResult(int requestCode, int resultCode, Intent intent) {
        if (requestCode == RC_SIGN_IN) {
            Task<GoogleSignInAccount> task =
                    GoogleSignIn.getSignedInAccountFromIntent(intent);
            try {
                GoogleSignInAccount account = task.getResult(ApiException.class);
                onConnected(account);
            } catch (ApiException apiException) {
                String message = apiException.getMessage();
                if (message == null || message.isEmpty()) {
                    message = "Sign in failed";
                } else if (message.length() < 5) {
                    message = "Sign in failed (" + message + ")";
                }

                onDisconnected();

                AppUtils.alertDebug(message);
            }
            return true;
        }
        return false;
    }

    private static void onConnected(GoogleSignInAccount account) {
        Games.getGamesClient(_activity, account).setViewForPopups(_layout);
        _achievements = Games.getAchievementsClient(_activity, account);
        _leaderboards = Games.getLeaderboardsClient(_activity, account);
        _isConnecting = false;
        _isConnected = true;
    }

    private static void onDisconnected() {
        _isConnecting = false;
        _isConnected = false;
        _achievements = null;
        _leaderboards = null;
    }
}
