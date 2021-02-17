package ek.admob;

import android.content.Context;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import com.google.android.gms.ads.AdError;
import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.LoadAdError;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.RequestConfiguration;
import com.google.android.gms.ads.rewarded.RewardItem;
import com.google.android.gms.ads.rewarded.RewardedAd;
import com.google.android.gms.ads.rewarded.RewardedAdCallback;
import com.google.android.gms.ads.rewarded.RewardedAdLoadCallback;

import java.util.Arrays;

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
    private RewardedAd _rewardedVideo;
    private InterstitialAd _interstitialAd;
    private int tagForChildDirectedTreatment;

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

    private void start() {
        _activity.runOnUiThread(() -> {
            final RequestConfiguration requestConfig = new RequestConfiguration.Builder()
                    .setTestDeviceIds(Arrays.asList(
                            AdRequest.DEVICE_ID_EMULATOR, // All emulators
                            "2AB46EC73CC840F948630EA11ECB6A3F",  // Galaxy A7
                            "3BA93F7574A4ECCF84878025F0B5F9D6"  // Galaxy S2
                    ))
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
                    _interstitialAd = new InterstitialAd(_activity);
                    _interstitialAd.setAdUnitId(_interstitialId);
                    _interstitialAd.loadAd(buildAdRequest());
                    _interstitialAd.setAdListener(new AdListener() {
                        @Override
                        public void onAdClosed() {
                            // Load the next interstitial.
                            postGLEvent(EVENT_INTERSTITIAL_CLOSED);

                            if (_interstitialAd != null) {
                                _interstitialAd.loadAd(buildAdRequest());
                            }
                        }

                        @Override
                        public void onAdFailedToLoad(int error) {
                            //Retry
                            _interstitialAd.loadAd(buildAdRequest());
                        }
                    });
                }

                if (_videoRewardId != null && !_videoRewardId.isEmpty()) {
                    _rewardedVideo = new RewardedAd(_context, _videoRewardId);
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
        if (_rewardedVideo != null && _videoRewardId != null && !_videoRewardId.isEmpty()) {
            _rewardedVideo.loadAd(buildAdRequest(), new RewardedAdLoadCallback() {
                public void onRewardedAdLoaded() {
                    postGLEvent(ADS_VIDEO_REWARD_LOADED);
                }

                public void onRewardedAdFailedToLoad(LoadAdError var1) {
                    postGLEvent(ADS_VIDEO_REWARD_FAIL);
                }
            });
        }
    }

    @Keep
    public static void show_interstitial_ad() {
        EkActivity.runMainThread(
                () -> {
                    if (instance._interstitialAd != null) {
                        if (instance._interstitialAd.isLoaded()) {
                            instance._interstitialAd.show();
                        } else {
                            postGLEvent(EVENT_INTERSTITIAL_CLOSED);
                        }
                    } else {
                        postGLEvent(EVENT_INTERSTITIAL_CLOSED);
                    }
                });
    }

    @Keep
    public static void show_rewarded_ad() {
        EkActivity.runMainThread(
                () -> {
                    if (instance._rewardedVideo != null) {
                        if (instance._rewardedVideo.isLoaded()) {
                            instance._rewardedVideo.show(instance._activity, new RewardedAdCallback() {
                                @Override
                                public void onUserEarnedReward(@NonNull RewardItem reward) {
                                    postGLEvent(ADS_VIDEO_REWARDED);
                                }

                                @Override
                                public void onRewardedAdClosed() {
                                    postGLEvent(ADS_VIDEO_REWARD_CLOSED);
                                }

                                @Override
                                public void onRewardedAdFailedToShow(AdError err) {
                                    postGLEvent(ADS_VIDEO_REWARD_FAIL);
                                }
                            });
                        } else {
                            instance.loadRewardedVideoAd();
                            postGLEvent(ADS_VIDEO_LOADING);
                        }
                    }
                });
    }

    @Keep
    public static void show_banner(final int flags) {
        EkActivity.runMainThread(
                () -> {
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
                }
        );
    }

    @Override
    public void onDestroy() {
        try {
            if (_banner != null) {
                _banner.destroy();
                _banner = null;
            }
            _interstitialAd = null;
            _rewardedVideo = null;
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
}
