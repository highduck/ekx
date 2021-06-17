#pragma once

#include <billing.hpp>
#include <ek/util/Platform.hpp>
#include <ek/scenex/app/GameAppListener.hpp>
#include <ek/util/Type.hpp>
#include <functional>
#include <memory>

namespace ek {

class AdMobWrapper;

class Ads : public GameAppListener {
public:

    struct Config {
        std::string skuRemoveAds = "remove_ads";
        std::string key0 = "ads_key_0";
        int val0 = 1111;
        std::string key1 = "ads_key_1";
        int val1 = 2222;
    };

    void onStart() override;

    explicit Ads(Config config);
    ~Ads() override = default;

    void onPurchaseChanged(const billing::PurchaseData& purchase);

    [[nodiscard]] bool isRemoved() const {
        return removed;
    }

    void purchaseRemoveAds() const;

    // TODO: rename, try start commercial break
    void gameOver(const std::function<void()>& callback);

    void showRewardVideo(const std::function<void(bool)>& callback);

    Signal<> onRemoved{};
    Signal<> onProductLoaded{};

    void cheat_RemoveAds();

    [[nodiscard]] bool hasVideoRewardSupport() const {
        return true;
    }

    std::string price{};

private:
    void setRemoveAdsPurchaseCache(bool adsRemoved) const;

    [[nodiscard]] bool checkRemoveAdsPurchase() const;

    void onRemoveAdsPurchased();

private:
    bool removed = false;
    Config config_;

    std::unique_ptr<AdMobWrapper> wrapper;
};

EK_DECLARE_TYPE(Ads);

}

