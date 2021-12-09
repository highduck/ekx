#pragma once

#include <admob.hpp>
#include <ek/ds/Pointer.hpp>

namespace ek {

class AdMobWrapper {
protected:
    bool isAvailable_ = true;
    bool activeInterstitial = false;

    std::function<void(bool)> rewardedAdCompletedCallback;
    bool userRewarded = false;
public:

    AdMobWrapper();

    virtual ~AdMobWrapper() = default;

    virtual void showInterstitial(std::function<void()> callback);

    virtual void showRewardedAd(std::function<void(bool)> callback);

    void completeRewardedAd(bool rewarded);

    void onAdmobEvent(admob::EventType event);

    [[nodiscard]]
    bool isAvailable() const {
        return isAvailable_;
    }

    static Pointer<AdMobWrapper> create(bool devMode);
};

}

