#include "AdMobNull.hpp"

namespace ek {

void AdMobNull::showInterstitial(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

void AdMobNull::showRewardedAd(const std::function<void(bool)>& callback) {
    if (callback) {
        callback(false);
    }
}
}