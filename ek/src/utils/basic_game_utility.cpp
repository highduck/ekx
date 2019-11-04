#include "basic_game_utility.hpp"

#include <cstdlib>
#include <ek/locator.hpp>
#include <platform/GameServices.h>
#include <platform/Sharing.h>
#include <platform/Ads.h>

namespace ek {

LeaderBoard::LeaderBoard(std::string id)
        : id_{std::move(id)} {

}

void LeaderBoard::show() const {
    leader_board_show(id_.c_str());
}

void LeaderBoard::submit(int score) const {
    leader_board_submit(id_.c_str(), score);
}

void BasicGameUtility::showAchievements() {
    achievement_show();
}

void BasicGameUtility::updateAchievement(const std::string& code, int count) {
    achievement_update(code.c_str(), count);
}

void BasicGameUtility::navigate(const std::string& url, bool blank) {
    (void)blank;
    sharing_navigate(url.c_str());
}

void BasicGameUtility::shareScore(const std::string& text) {
    sharing_send_message(text.c_str());
}

void BasicGameUtility::shareApp(const std::string& text) {
    sharing_send_message(text.c_str());
}

void BasicGameUtility::rateUs(const std::string& appId) {
    sharing_rate_us(appId.c_str());
}

int BasicGameUtility::unlockSite() { return 1; }


/////////////
signal_t<> BasicGameUtility::onAdsRemoved;

signal_t<> BasicGameUtility::onAdsRewarded;

static std::function<void(bool)> sShowVideoCallback;
static bool sIsAdsRemoved = false;

void BasicGameUtility::adsOnGameOver() {
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
            BasicGameUtility::onAdsRewarded();
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
            BasicGameUtility::onAdsRemoved();
            break;

        default:

            break;
    }
}

void BasicGameUtility::adsShowVideo(const std::function<void(bool)>& callback) {
    sShowVideoCallback = callback;
    ads_play_reward_video();
}

void BasicGameUtility::removeAds() {
    ads_purchase_remove();
}

bool BasicGameUtility::isAdsRemoved() {
    return sIsAdsRemoved;
}

void BasicGameUtility::init() {
    ads_listen(onAdsEvents);
}

}
