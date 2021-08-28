package ek.admob;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.provider.Settings;
import android.util.Log;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.FullScreenContentCallback;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.OnUserEarnedRewardListener;
import com.google.android.gms.ads.RequestConfiguration;
import com.google.android.gms.ads.interstitial.InterstitialAd;
import com.google.android.gms.ads.interstitial.InterstitialAdLoadCallback;
import com.google.android.gms.ads.rewarded.RewardItem;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;

import ek.AppUtils;
import ek.EkActivity;
import ek.EkPlugin;
import ek.EkPluginManager;

@Keep
public class AdMobPlugin extends EkPlugin {

    final static String TAG = "AdMobPlugin";

    final static int INITIALIZED = 0;
    final static int ADS_VIDEO_REWARDED = 1;
    final static int ADS_VIDEO_REWARD_LOADED = 2;
    final static int ADS_VIDEO_REWARD_FAIL = 3;
    final static int ADS_VIDEO_LOADING = 4;
    final static int ADS_VIDEO_REWARD_CLOSED = 5;

    final static int EVENT_INTERSTITIAL_CLOSED = 6;

    private final ViewGroup _layout;
    private final Context _context;
    private final EkActivity _activity;

    private final String _bannerId;
    private final String _videoRewardId;
    private final String _interstitialId;

    private AdView _banner;
    private RewardedAd _rewardedAd;
    private InterstitialAd _interstitialAd;
    private boolean _interstitialAdLoading = false;
    final private int tagForChildDirectedTreatment;

    static AdMobPlugin instance;

    public AdMobPlugin(final String banner,
                       final String video,
                       final String interstitial,
                       final int tagForChildDirectedTreatment) {
        _activity = EkActivity.getInstance();

        _context = _activity;
        _layout = _activity.mainLayout;

        _bannerId = banner;
        _interstitialId = interstitial;
        _videoRewardId = video;
        this.tagForChildDirectedTreatment = tagForChildDirectedTreatment;
    }

    static int convertAdMobTagForChildDirectedTreatment(int v) {
        if (v == 0) {
            return RequestConfiguration.TAG_FOR_CHILD_DIRECTED_TREATMENT_FALSE;
        } else if (v == 1) {
            return RequestConfiguration.TAG_FOR_CHILD_DIRECTED_TREATMENT_TRUE;
        }
        return RequestConfiguration.TAG_FOR_CHILD_DIRECTED_TREATMENT_UNSPECIFIED;
    }

    private void loadInterstitialAd() {
        _interstitialAd = null;
        _interstitialAdLoading = true;
        InterstitialAd.load(_activity, _interstitialId, buildAdRequest(), new InterstitialAdLoadCallback() {
            @Override
            public void onAdLoaded(@NonNull InterstitialAd interstitialAd) {
                // The mInterstitialAd reference will be null until
                // an ad is loaded.
                Log.i(TAG, "onAdLoaded");
                _interstitialAdLoading = false;
                _interstitialAd = interstitialAd;
                _interstitialAd.setFullScreenContentCallback(new FullScreenContentCallback() {
                    @Override
                    public void onAdDismissedFullScreenContent() {
                        // Called when fullscreen content is dismissed.
                        Log.d("TAG", "The ad was dismissed.");

                        postGLEvent(EVENT_INTERSTITIAL_CLOSED);

                        // load next interstitial
                        loadInterstitialAd();
                    }

                    @Override
                    public void onAdFailedToShowFullScreenContent(AdError adError) {
                        // Called when fullscreen content failed to show.
                        Log.d("TAG", "The ad failed to show.");

                        postGLEvent(EVENT_INTERSTITIAL_CLOSED);

                        // load next interstitial
                        loadInterstitialAd();
                    }

                    @Override
                    public void onAdShowedFullScreenContent() {
                        // Called when fullscreen content is shown.
                        // Make sure to set your reference to null so you don't
                        // show it a second time.
                        _interstitialAd = null;
                        Log.d("TAG", "The ad was shown.");
                    }
                });
            }

            @Override
            public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
                // Handle the error
                Log.i(TAG, loadAdError.getMessage());
                _interstitialAdLoading = false;
                _interstitialAd = null;
                // wait until next request
                //loadInterstitialAd();
            }
        });
    }

    private void start() {
        _activity.runOnUiThread(() -> {
            //UMP.start();
            final RequestConfiguration requestConfig = new RequestConfiguration.Builder()
                    .setTestDeviceIds(getTestDeviceIds(_activity))
                    .setTagForChildDirectedTreatment(tagForChildDirectedTreatment)
                    .build();
            MobileAds.setRequestConfiguration(requestConfig);
            MobileAds.initialize(_context, status -> {
                if (_bannerId != null && !_bannerId.isEmpty()) {
                    _banner = new AdView(_context);
                    RelativeLayout.LayoutParams params =
                            new RelativeLayout.LayoutParams(
                                    RelativeLayout.LayoutParams.WRAP_CONTENT,
                                    RelativeLayout.LayoutParams.WRAP_CONTENT
                            );
                    params.addRule(RelativeLayout.CENTER_HORIZONTAL);
                    params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                    _banner.setLayoutParams(params);
                    _banner.setAdSize(AdSize.SMART_BANNER);
                    _banner.setAdUnitId(_bannerId);
                    _banner.loadAd(buildAdRequest());
                }

                if (_interstitialId != null && !_interstitialId.isEmpty()) {
                    loadInterstitialAd();
                }

                if (_videoRewardId != null && !_videoRewardId.isEmpty()) {
                    loadRewardedVideoAd();
                }

                postGLEvent(INITIALIZED);
            });
        });
    }

    private static AdRequest buildAdRequest() {
        return new AdRequest.Builder().build();
    }

    @Keep
    public static native void eventCallback(int event);

    @Keep
    public static void initialize(final String banner,
                                  final String video,
                                  final String interstitial,
                                  final int tagForChildDirectedTreatment) {
        instance = new AdMobPlugin(banner, video, interstitial,
                convertAdMobTagForChildDirectedTreatment(tagForChildDirectedTreatment)
        );
        EkPluginManager.instance.extensions.add(instance);
        instance.start();
    }

    public void loadRewardedVideoAd() {
        if (_videoRewardId == null || _videoRewardId.isEmpty()) {
            return;
        }
        _rewardedAd = null;
        RewardedAd.load(_activity, _videoRewardId, buildAdRequest(), new RewardedAdLoadCallback() {
            @Override
            public void onAdFailedToLoad(@NonNull LoadAdError loadAdError) {
                _rewardedAd = null;
                postGLEvent(ADS_VIDEO_REWARD_FAIL);
            }

            @Override
            public void onAdLoaded(@NonNull RewardedAd rewardedAd) {
                Log.d(TAG, "onAdFailedToLoad");
                _rewardedAd = rewardedAd;
                _rewardedAd.setFullScreenContentCallback(new FullScreenContentCallback() {
                    @Override
                    public void onAdShowedFullScreenContent() {
                        // Called when ad is shown.
                        Log.d(TAG, "Rewarded Ad was shown");
                        _rewardedAd = null;
                    }

                    @Override
                    public void onAdFailedToShowFullScreenContent(AdError adError) {
                        // Called when ad fails to show.
                        Log.d(TAG, "Rewarded Ad failed to show");
                        postGLEvent(ADS_VIDEO_REWARD_FAIL);
                    }

                    @Override
                    public void onAdDismissedFullScreenContent() {
                        // Called when ad is dismissed.
                        // Don't forget to set the ad reference to null so you
                        // don't show the ad a second time.
                        Log.d(TAG, "Rewarded Ad was dismissed");
                        postGLEvent(ADS_VIDEO_REWARD_CLOSED);
                    }
                });
                postGLEvent(ADS_VIDEO_REWARD_LOADED);
            }
        });
    }

    @Keep
    public static void show_interstitial_ad() {
        EkActivity.runMainThread(() -> {
            if (instance._interstitialAd != null) {
                // ad is ready to go
                instance._interstitialAd.show(instance._activity);
            } else {
                postGLEvent(EVENT_INTERSTITIAL_CLOSED);
                if (!instance._interstitialAdLoading) {
                    instance.loadInterstitialAd();
                }
            }
        });
    }

    @Keep
    public static void show_rewarded_ad() {
        EkActivity.runMainThread(() -> {
            if (instance._rewardedAd != null) {
                instance._rewardedAd.show(instance._activity, new OnUserEarnedRewardListener() {
                    @Override
                    public void onUserEarnedReward(@NonNull RewardItem rewardItem) {
                        // Handle the reward.
                        Log.d(TAG, "The user earned the reward");
                        //int rewardAmount = rewardItem.getAmount();
                        //String rewardType = rewardItem.getType();
                        postGLEvent(ADS_VIDEO_REWARDED);
                    }
                });
            } else {
                instance.loadRewardedVideoAd();
                postGLEvent(ADS_VIDEO_LOADING);
            }
        });
    }

    @Keep
    public static void show_banner(final int flags) {
        EkActivity.runMainThread(() -> {
            if (instance._banner != null) {
                if (flags != 0) {
                    if (instance._banner.getParent() == null) {
                        instance._layout.addView(instance._banner);
                    }
                } else {
                    if (instance._banner.getParent() != null) {
                        instance._layout.removeView(instance._banner);
                    }
                }
            }
        });
    }

    @Override
    public void onDestroy() {
        try {
            if (_banner != null) {
                _banner.destroy();
                _banner = null;
            }
            _interstitialAd = null;
            _rewardedAd = null;
        } catch (Exception ignored) {
        }
    }

    @Override
    public void onPause() {
        try {
            if (_banner != null) {
                _banner.pause();
            }
        } catch (Exception ignored) {
        }
    }

    @Override
    public void onResume(boolean inFocus) {
        try {
            if (_banner != null) {
                _banner.resume();
            }
        } catch (Exception ignored) {
        }
    }

    public static void postGLEvent(final int event) {
        EkActivity.runGLThread(() -> eventCallback(event));
    }

    /**
     * For Test devices, TestLab, debugging
     **/

    static ArrayList<String> getTestDeviceIds(final Activity activity) {
        final ArrayList<String> ids = new ArrayList<>();

        ids.add(AdRequest.DEVICE_ID_EMULATOR); // All emulators
        // TODO: configuration
        ids.add("2AB46EC73CC840F948630EA11ECB6A3F"); // Galaxy A7
        ids.add("3BA93F7574A4ECCF84878025F0B5F9D6"); // Galaxy S2

        if (AppUtils.isTestLab() || AppUtils.isDebugBuild()) {
            final String deviceID = getCurrentDeviceID(activity);
            if (!deviceID.isEmpty()) {
                ids.add(deviceID);
            }
        }
        return ids;
    }

    private static String getCurrentDeviceID(final Activity activity) {
        final ContentResolver resolver = activity.getContentResolver();
        @SuppressLint("HardwareIds") final String androidID = Settings.Secure.getString(resolver, Settings.Secure.ANDROID_ID);
        if (androidID != null && androidID.length() > 0) {
            return md5(androidID);
        }
        return "";
    }

    private static String md5(@NonNull final String s) {
        try {
            final MessageDigest digest = java.security.MessageDigest.getInstance("MD5");
            digest.update(s.getBytes());
            return hex(digest.digest());
        } catch (NoSuchAlgorithmException e) {
            //Logger.logStackTrace(TAG, e);
        }
        return "";
    }

    private static String hex(@NonNull final byte[] bytes) {
        StringBuilder builder = new StringBuilder(32 + 16);
        final char[] hexDigits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        for (byte b : bytes) {
            builder.append(hexDigits[(b >>> 4) & 0xF]);
            builder.append(hexDigits[b & 0xF]);
        }
        return builder.toString();
    }
}
