#include "mini_ads.hpp"

namespace ek::mini_ads {

//signal_t<> onAdsRemoved;
//
//signal_t<> onAdsRewarded;

static std::function<void(bool)> sShowVideoCallback;
static bool sIsAdsRemoved = false;

void adsOnGameOver() {
    if (!sIsAdsRemoved) {
        ads_show_interstitial();
    }
}

static void onAdsEvents(AdsEventType type) {
    switch (type) {
        case AdsEventType::ADS_VIDEO_REWARDED:
            if (sShowVideoCallback) {
                auto pr = sShowVideoCallback;
                sShowVideoCallback = std::function<void(bool)>();
                pr(true);
            }
            onAdsRewarded();
            break;

        case AdsEventType::ADS_VIDEO_LOADING:
            break;

        case AdsEventType::ADS_VIDEO_REWARD_FAIL:
            if (sShowVideoCallback) {
                auto pr = sShowVideoCallback;
                sShowVideoCallback = std::function<void(bool)>();
                pr(false);
            }
            break;
        case AdsEventType::ADS_VIDEO_REWARD_CLOSED:
            if (sShowVideoCallback) {
                auto pr = sShowVideoCallback;
                sShowVideoCallback = std::function<void(bool)>();
                pr(false);
            }
            break;
        case AdsEventType::ADS_VIDEO_REWARD_LOADED:
            if (sShowVideoCallback) {
//                auto pr = sShowVideoCallback;
//                sShowVideoCallback = std::function<void(bool)>();
//                pr(true);
                ads_play_reward_video();
            }
            break;

        case AdsEventType::ADS_REMOVED:
            sIsAdsRemoved = true;
            onAdsRemoved();
            break;

        default:

            break;
    }
}

void adsShowVideo(const std::function<void(bool)>& callback) {
    sShowVideoCallback = callback;
    ads_play_reward_video();
}

void removeAds() {
    ads_purchase_remove();
}

bool isAdsRemoved() {
    return sIsAdsRemoved;
}

void init() {
    ads_listen(onAdsEvents);
}

}