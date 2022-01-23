#pragma once

#include <ek/admob.h>
#include <functional>
#include <ek/ds/Pointer.hpp>

namespace ek {

class AdMobWrapper {
protected:
    bool isAvailable_ = true;

    std::function<void()> interstitialCompletedCallback;
    bool activeInterstitial = false;

    std::function<void(bool)> rewardedAdCompletedCallback;
    bool userRewarded = false;
public:

    AdMobWrapper();

    virtual ~AdMobWrapper() = default;

    virtual void showInterstitial(std::function<void()> callback);

    virtual void showRewardedAd(std::function<void(bool)> callback);

    void completeRewardedAd(bool rewarded);

    void onAdmobEvent(ek_admob_event_type event);

    [[nodiscard]]
    bool isAvailable() const {
        return isAvailable_;
    }

    static AdMobWrapper* create(bool devMode);
};

}

