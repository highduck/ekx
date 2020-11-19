#pragma once

#include <ek/app/prefs.hpp>
#include <billing.hpp>
#include <admob.hpp>
#include <ek/util/detect_platform.hpp>
#include <functional>
#include <memory>
#include <AdMobWrapper.hpp>
#include <utility>

#if EK_EDITOR
const bool useAdMobSimulator = true;
#else
const bool useAdMobSimulator = false;
#endif

namespace ek {

class Ads {

public:
    struct Config {
        std::string skuRemoveAds = "remove_ads";
        std::string key0 = "ads_key_0";
        int val0 = 1111;
        std::string key1 = "ads_key_1";
        int val1 = 2222;
    };

    signal_t<> onRemoved{};

    explicit Ads(Config config);

    void onPurchaseChanged(const billing::PurchaseData& purchase);

    [[nodiscard]]
    bool isRemoved() const {
        return removed;
    }

    void purchaseRemoveAds() const {
        billing::purchase(config_.skuRemoveAds, "");
    }

    // TODO: rename, try start commercial break
    void gameOver(const std::function<void()>& callback) {
        if (removed) {
            if (callback) {
                callback();
            }
        } else {
            wrapper->showInterstitial(callback);
        }
    }

    void showRewardVideo(const std::function<void(bool)>& callback) {
        wrapper->showRewardedAd(callback);
    }

private:
    void clearRemoveAdsPurchase() const {
        set_user_preference(config_.key0, 0);
        set_user_preference(config_.key1, 1);
    }

    void cacheRemoveAdsPurchase() const {
        set_user_preference(config_.key0, config_.val0);
        set_user_preference(config_.key1, config_.val1);
    }

    [[nodiscard]]
    bool checkRemoveAdsPurchase() const {
        return get_user_preference(config_.key0, 0) == config_.val0 &&
               get_user_preference(config_.key1, 0) == config_.val1;
    }

    void onRemoveAdsPurchased() {
        // disable current ads
        removed = true;
        //AdsController::disableAds()

        // save
        cacheRemoveAdsPurchase();

        // dispatch event that ads is removed
        onRemoved();
    }

private:
    bool removed = false;
    Config config_;

    std::unique_ptr<AdMobWrapper> wrapper{AdMobWrapper::create(useAdMobSimulator)};
};

}

