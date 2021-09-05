#pragma once

#include <admob.hpp>

#include <memory>

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

    virtual void showInterstitial(const std::function<void()>& callback);

    virtual void showRewardedAd(const std::function<void(bool)>& callback);

    void completeRewardedAd(bool rewarded);

    void onAdmobEvent(admob::EventType event);

    [[nodiscard]]
    bool isAvailable() const {
        return isAvailable_;
    }

    static std::unique_ptr<AdMobWrapper> create(bool devMode);
};

}

