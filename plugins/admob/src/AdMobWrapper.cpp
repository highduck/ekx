#include "AdMobWrapper.hpp"
#include <ek/audio/audio.hpp>
#include "AdMobNull.hpp"

// implementation
#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

#if defined(__ANDROID__) || TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#define EK_ADMOB_SIMULATOR 0
#else
#define EK_ADMOB_SIMULATOR 1
#endif

#if EK_ADMOB_SIMULATOR

#include "AdMobSimulator.hpp"

#endif

namespace ek {

AdMobWrapper::AdMobWrapper() {
    admob::context.onEvent += [this](auto event) {
        this->onAdmobEvent(event);
    };
}

void AdMobWrapper::showInterstitial(std::function<void()> callback) {
    activeInterstitial = true;
    audio::muteDeviceBegin();
    admob::context.onInterstitialClosed.once([cb=std::move(callback), this] {
        activeInterstitial = false;
        audio::muteDeviceEnd();
        if (cb) {
            cb();
        }
    });
    admob::show_interstitial_ad();
}

void AdMobWrapper::showRewardedAd(std::function<void(bool)> callback) {
    rewardedAdCompletedCallback = std::move(callback);
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

void AdMobWrapper::onAdmobEvent(admob::EventType event) {
    using namespace admob;
    switch (event) {
        case EventType::VideoRewarded:
            userRewarded = true;
            break;
            case EventType::VideoFailed:
                case EventType::VideoClosed:
            completeRewardedAd(userRewarded);
            break;
            case EventType::VideoLoaded:
            if (rewardedAdCompletedCallback) {
                show_rewarded_ad();
            }
            break;
        default:
            break;
    }
}

Pointer<AdMobWrapper> AdMobWrapper::create(bool devMode) {
    (void)devMode;

#if EK_ADMOB_SIMULATOR
    if (devMode) {
        return Pointer<AdMobSimulator>::make();
    }
#endif

    if (admob::hasSupport()) {
        return Pointer<AdMobWrapper>::make();
    }
    return Pointer<AdMobNull>::make();
}

}
