#pragma once

#include "AdMobWrapper.hpp"

namespace ek {

class AdMobNull : public AdMobWrapper {
public:

    void showInterstitial(std::function<void()> callback) override {
        if (callback) {
            callback();
        }
    }

    void showRewardedAd(std::function<void(bool)> callback) override {
        if (callback) {
            callback(false);
        }
    }
};

}

