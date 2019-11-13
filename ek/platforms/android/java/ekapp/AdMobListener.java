package ekapp;

import com.google.android.gms.ads.reward.RewardItem;
import com.google.android.gms.ads.reward.RewardedVideoAdListener;

public class AdMobListener implements RewardedVideoAdListener {

    AdMobListener() {
    }

    @Override
    public void onRewardedVideoAdLoaded() {
        Ads.postGLEvent(Ads.ADS_VIDEO_REWARD_LOADED);
    }

    @Override
    public void onRewardedVideoAdOpened() {

    }

    @Override
    public void onRewardedVideoStarted() {

    }

    @Override
    public void onRewardedVideoAdClosed() {
        Ads.postGLEvent(Ads.ADS_VIDEO_REWARD_CLOSED);

        // Load the next rewarded video ad.
        Ads.loadRewardedVideoAd();
    }

    @Override
    public void onRewarded(RewardItem rewardItem) {
        Ads.postGLEvent(Ads.ADS_VIDEO_REWARDED);
    }

    @Override
    public void onRewardedVideoAdLeftApplication() {

    }

    @Override
    public void onRewardedVideoAdFailedToLoad(int i) {
        Ads.postGLEvent(Ads.ADS_VIDEO_REWARD_FAIL);
    }

    @Override
    public void onRewardedVideoCompleted() {

    }
}
