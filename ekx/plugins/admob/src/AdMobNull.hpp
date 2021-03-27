#pragma once

#include "AdMobWrapper.hpp"

namespace ek {

class AdMobNull : public AdMobWrapper {
public:

    void showInterstitial(const std::function<void()>& callback) override;

    void showRewardedAd(const std::function<void(bool)>& callback) override;
};

}

