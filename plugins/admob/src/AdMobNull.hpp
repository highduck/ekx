#pragma once

#include "AdMobWrapper.hpp"

namespace ek {

class AdMobNull : public AdMobWrapper {
public:

    void showInterstitial(const std::function<void()>& callback) override {
        if (callback) {
            callback();
        }
    }

    void showRewardedAd(const std::function<void(bool)>& callback) override {
        if (callback) {
            callback(false);
        }
    }
};

}

