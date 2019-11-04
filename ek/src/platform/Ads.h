#pragma once

#include <functional>
#include <string>

namespace ek {

enum class AdsEventType {
    ADS_VIDEO_REWARDED = 1,
    ADS_VIDEO_REWARD_LOADED = 2,
    ADS_VIDEO_REWARD_FAIL = 3,
    ADS_VIDEO_LOADING = 4,
    ADS_VIDEO_REWARD_CLOSED = 5,
    ADS_REMOVED = 6
};

struct ads_config_t {
    std::string app_id;
    std::string banner;
    std::string video;
    std::string inters;
    std::string remove_ads_sku;
};

void ads_init(const ads_config_t& config);

void ads_reset_purchase();

void ads_set_banner(int flags);

void ads_play_reward_video();

void ads_show_interstitial();

void ads_listen(const std::function<void(AdsEventType type)>& callback);

void ads_purchase_remove();

void init_billing(const char* key);

}