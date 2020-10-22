package ek;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import androidx.annotation.Keep;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.reward.RewardedVideoAd;

@Keep
public class Ads implements EkExtension {

    final private static String TAG = "Ads";

    public final static int ADS_VIDEO_REWARDED = 1;
    public static final int ADS_VIDEO_REWARD_LOADED = 2;
    public static final int ADS_VIDEO_REWARD_FAIL = 3;
    public static final int ADS_VIDEO_LOADING = 4;
    public static final int ADS_VIDEO_REWARD_CLOSED = 5;
    public static final int ADS_REMOVED = 6;

    private static ViewGroup _layout;
    private static Context _context;
    private static EkActivity _activity;

    private static String _appId;
    private static String _bannerId;
    private static String _videoRewardId;
    private static String _interstitialId;

    private static AdView _banner;
    private static RewardedVideoAd _rewardedVideo;
    private static AdMobListener _listener;
    private static InterstitialAd _interstitialAd;
    private static boolean _adsRemoved = false;

    public static Ads extension;

    public Ads() {

    }

    private static AdRequest buildAdRequest() {
        return new AdRequest.Builder()
                .addTestDevice(AdRequest.DEVICE_ID_EMULATOR)        // All emulators
                .addTestDevice("2AB46EC73CC840F948630EA11ECB6A3F")  // Galaxy A7
                .addTestDevice("3BA93F7574A4ECCF84878025F0B5F9D6")  // Galaxy S2
                .addTestDevice("58D245E62960E58D4134E35BA336CB88")  // Redmi Note 3
                .addTestDevice("10D31ECCA5FEDBE0AFF39D2549EFABC7")  // Redmi 7
                .build();
    }

    @Keep
    public static native void eventCallback(int event);

    @Keep
    public static void reset_purchase() {
        Preferences.set_int("noads", 0);
        Preferences.set_int("_bm9hZHM=", 1);
    }

    @Keep
    public static void init(final String appId, final String banner, final String video, final String interstitial, final String remove_ads_sku) {
        extension = new Ads();
        EkExtensionManager.instance.extensions.add(extension);

        Log.d(TAG, "register");
        _activity = EkActivity.getInstance();
        _context = _activity;
        _layout = _activity.mainLayout;

        _appId = appId;
        _bannerId = banner;
        _interstitialId = interstitial;
        _videoRewardId = video;

        RemoveAds.instance.start(remove_ads_sku);

        _activity.runOnUiThread(
                new Runnable() {
                    @Override
                    public void run() {
                        MobileAds.initialize(_context, _appId);
                        if (Preferences.get_int("noads", 0) == 310 && Preferences.get_int("_bm9hZHM=", 1) == 1988) {
                            _adsRemoved = true;
                            _bannerId = null;
                            _interstitialId = null;
                            postGLEvent(ADS_REMOVED);
                        }

                        _listener = new AdMobListener();
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
                                    if (_interstitialAd != null) {
                                        _interstitialAd.loadAd(buildAdRequest());
                                    }
                                }
                            });
                        }

                        if (_videoRewardId != null && !_videoRewardId.isEmpty()) {
                            _rewardedVideo = MobileAds.getRewardedVideoAdInstance(_context);
                            _rewardedVideo.setRewardedVideoAdListener(_listener);
                            loadRewardedVideoAd();
                        }
                    }
                });
    }

    public static void loadRewardedVideoAd() {
        if (_rewardedVideo != null && _videoRewardId != null && !_videoRewardId.isEmpty()) {
            _rewardedVideo.loadAd(_videoRewardId, buildAdRequest());
        }
    }

    @Keep
    public static void show_interstitial() {
        EkActivity.runMainThread(
                new Runnable() {
                    @Override
                    public void run() {
                        if (_interstitialAd != null && _interstitialAd.isLoaded()) {
                            _interstitialAd.show();
                        }
                    }
                });
    }

    @Keep
    public static void play_reward_video() {
        EkActivity.runMainThread(
                new Runnable() {
                    @Override
                    public void run() {
                        if (_rewardedVideo != null) {
                            if (_rewardedVideo.isLoaded()) {
                                _rewardedVideo.show();
                            } else {
                                loadRewardedVideoAd();
                                postGLEvent(ADS_VIDEO_LOADING);
                            }
                        }
                    }
                });
    }

    @Keep
    public static void set_banner(final int flags) {
        EkActivity.runMainThread(
                new Runnable() {
                    @Override
                    public void run() {
                        if (_banner != null) {
                            if (flags != 0) {
                                if (_banner.getParent() == null) {
                                    _layout.addView(_banner);
                                }
                            } else {
                                if (_banner.getParent() != null) {
                                    _layout.removeView(_banner);
                                }
                            }
                        }
                    }
                }
        );
    }

    @Keep
    public static void purchase_remove() {
        RemoveAds.instance.purchaseRemoveAds();
    }

    public static void destroy() {
        try {
            if (_banner != null) {
                _banner.destroy();
                _banner = null;
            }
            if (_rewardedVideo != null) {
                _rewardedVideo.destroy(_context);
                _rewardedVideo = null;
            }
        } catch (Exception ignored) {
        }
    }

    public static void postGLEvent(final int event) {
        EkActivity.runGLThread(new Runnable() {
            @Override
            public void run() {
                Ads.eventCallback(event);
            }
        });
    }

    public static void removeAds() {
        _adsRemoved = true;
        _bannerId = null;
        _interstitialId = null;
        Preferences.set_int("noads", 310);
        Preferences.set_int("_bm9hZHM=", 1988);

        if (_banner != null) {
            try {
                if (_banner.getParent() != null) {
                    _layout.removeView(_banner);
                }
                _banner.destroy();
            } catch (Exception ignored) {

            }
            _banner = null;
        }

        if (_interstitialAd != null) {
            _interstitialAd = null;
        }

        postGLEvent(ADS_REMOVED);
    }

    @Override
    public void onApplicationStart() {

    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent intent) {

    }

    @Override
    public void onApplicationPause() {
        try {
            if (_banner != null) {
                _banner.pause();
            }
            if (_rewardedVideo != null) {
                _rewardedVideo.pause(_context);
            }
        } catch (Exception ignored) {
        }
    }

    @Override
    public void onApplicationResume(boolean inFocus) {
        try {
            if (_banner != null) {
                _banner.resume();
            }
            if (_rewardedVideo != null) {
                _rewardedVideo.resume(_context);
            }
        } catch (Exception ignored) {
        }
    }
}
