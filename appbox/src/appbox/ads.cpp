#include "ads.hpp"

namespace ek {

Ads::Ads(Ads::Config config) :
        config_{std::move(config)} {
    billing::onPurchaseChanged += [this](auto purchase) { this->onPurchaseChanged(purchase); };

    removed = checkRemoveAdsPurchase();
    if (!removed) {
        billing::getPurchases();
        billing::getDetails({config_.skuRemoveAds});
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
}