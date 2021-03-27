#include "AdMobWrapper.hpp"

#include "AdMobSimulator.hpp"
#include "AdMobNull.hpp"

#include <ek/audio/audio.hpp>

#include <memory>

namespace ek {

AdMobWrapper::AdMobWrapper() {
    admob::context.onEvent += [this](auto event) {
        this->onAdmobEvent(event);
    };
}

void AdMobWrapper::showInterstitial(const std::function<void()>& callback) {
    activeInterstitial = true;
    audio::muteDeviceBegin();
    admob::context.onInterstitialClosed.add_once([callback, this] {
        activeInterstitial = false;
        audio::muteDeviceEnd();
        if (callback) {
            callback();
        }
    });
    admob::show_interstitial_ad();
}

void AdMobWrapper::showRewardedAd(const std::function<void(bool)>& callback) {
    rewardedAdCompletedCallback = callback;
    userRewarded = false;
    admob::show_rewarded_ad();
    audio::muteDeviceBegin();
}

void AdMobWrapper::completeRewardedAd(bool rewarded) {
    if (rewardedAdCompletedCallback) {
        audio::muteDeviceEnd();
        rewardedAdCompletedCallback(rewarded);
        rewardedAdCompletedCallback = nullptr;
    }
}

void AdMobWrapper::onAdmobEvent(admob::event_type event) {
    using namespace admob;
    switch (event) {
        case event_type::video_rewarded:
            userRewarded = true;
            break;
        case event_type::video_failed:
        case event_type::video_closed:
            completeRewardedAd(userRewarded);
            break;
        case event_type::video_loaded:
            if (rewardedAdCompletedCallback) {
                show_rewarded_ad();
            }
            break;
        default:
            break;
    }
}

std::unique_ptr<AdMobWrapper> AdMobWrapper::create(bool devMode) {
    if(devMode) {
        return std::make_unique<AdMobSimulator>();
    }
    else if(admob::hasSupport()) {
        return std::make_unique<AdMobWrapper>();
    }
    return std::make_unique<AdMobNull>();
}

}
