#pragma once

#include "AdMobWrapper.hpp"
#include <ek/scenex/app/GameAppListener.hpp>

namespace ek {

class AdMobSimulator : public AdMobWrapper, public GameAppListener {

    bool activeRewardedAd = false;

public:

    void onRenderFrame() override;

    AdMobSimulator();

    ~AdMobSimulator() override;

    void showInterstitial(const std::function<void()>& callback) override;

    void showRewardedAd(const std::function<void(bool)>& callback) override;
};

}

