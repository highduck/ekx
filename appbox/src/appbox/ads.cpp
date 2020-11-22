#include <ek/timers.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include "ads.hpp"

namespace ek {

Ads::Ads(Ads::Config config) :
        config_{std::move(config)} {
    billing::onPurchaseChanged += [this](auto purchase) { this->onPurchaseChanged(purchase); };

#ifndef NDEBUG
    clearRemoveAdsPurchase();
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

}