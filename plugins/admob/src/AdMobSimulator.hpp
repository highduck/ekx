#pragma once

#include "AdMobWrapper.hpp"

namespace ek {

class AdMobSimulator : public AdMobWrapper {

    bool activeRewardedAd = false;

    void draw() const;

public:

    AdMobSimulator();

    void showInterstitial(const std::function<void()>& callback) override;

    void showRewardedAd(const std::function<void(bool)>& callback) override;
};

}

