#pragma once

#include <billing.hpp>
#include <ek/util/detect_platform.hpp>
#include <functional>
#include <memory>

namespace ek {

class AdMobWrapper;

class Ads {
public:

    struct Config {
        std::string skuRemoveAds = "remove_ads";
        std::string key0 = "ads_key_0";
        int val0 = 1111;
        std::string key1 = "ads_key_1";
        int val1 = 2222;
    };

    explicit Ads(Config config);

    void onPurchaseChanged(const billing::PurchaseData& purchase);

    [[nodiscard]] bool isRemoved() const {
        return removed;
    }

    void purchaseRemoveAds() const;

    // TODO: rename, try start commercial break
    void gameOver(const std::function<void()>& callback);

    void showRewardVideo(const std::function<void(bool)>& callback);

    signal_t<> onRemoved{};

private:
    void setRemoveAdsPurchaseCache(bool reset) const;

    [[nodiscard]] bool checkRemoveAdsPurchase() const;

    void onRemoveAdsPurchased();

private:
    bool removed = false;
    Config config_;

    std::unique_ptr<AdMobWrapper> wrapper;
};

}

