#include "Ads.hpp"

#include <ek/timers.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/app/prefs.hpp>
#include <AdMobWrapper.hpp>

namespace ek {

#if EK_DEV_TOOLS
const bool useAdMobSimulator = true;
#else
const bool useAdMobSimulator = false;
#endif

Ads::Ads(Ads::Config config) :
        config_{std::move(config)},
        wrapper{AdMobWrapper::create(useAdMobSimulator)} {
    billing::context.onPurchaseChanged += [this](auto purchase) { this->onPurchaseChanged(purchase); };

#ifndef NDEBUG
    setRemoveAdsPurchaseCache(true);
#endif

    removed = checkRemoveAdsPurchase();
    if (!removed) {
        // just wait billing service a little, TODO: billing initialized promise
        resolve<basic_application>().onStartHook << [sku = config_.skuRemoveAds] {
            setTimeout([sku] {
                billing::getPurchases();
                billing::getDetails({sku});
            }, 3);
        };
    }
}

void Ads::onPurchaseChanged(const billing::PurchaseData& purchase) {
    if (!removed && purchase.productID == config_.skuRemoveAds && purchase.state == 0) {
        onRemoveAdsPurchased();
        if (!purchase.token.empty()) {
            // non-consumable
        }
    }
}

void Ads::purchaseRemoveAds() const {
    billing::purchase(config_.skuRemoveAds, "");
}

void Ads::setRemoveAdsPurchaseCache(bool reset) const {
    set_user_preference(config_.key0.c_str(), reset ? 0 : config_.val0);
    set_user_preference(config_.key1.c_str(), reset ? 1 : config_.val1);
}

bool Ads::checkRemoveAdsPurchase() const {
    return get_user_preference(config_.key0.c_str(), 0) == config_.val0 &&
           get_user_preference(config_.key1.c_str(), 0) == config_.val1;
}

void Ads::onRemoveAdsPurchased() {
    // disable current ads
    removed = true;

    // save
    setRemoveAdsPurchaseCache(false);

    // dispatch event that ads is removed
    onRemoved();
}

void Ads::gameOver(const std::function<void()>& callback) {
    if (removed) {
        if (callback) {
            callback();
        }
    } else {
        wrapper->showInterstitial(callback);
    }
}

void Ads::showRewardVideo(const std::function<void(bool)>& callback) {
    wrapper->showRewardedAd(callback);
}

}