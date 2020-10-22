#pragma once

#include <functional>
#include <string>
#include <ek/util/signals.hpp>

namespace ek {

enum class ads_event_type {
    video_rewarded = 1,
    video_loaded = 2,
    video_failed = 3,
    video_loading = 4,
    video_closed = 5,
    removed = 6
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

void ads_listen(const std::function<void(ads_event_type type)>& callback);

void ads_purchase_remove();

void init_billing(const char* key);


////
namespace mini_ads {

using show_video_callback = std::function<void(bool)>;

static signal_t<> on_removed{};
static signal_t<> on_video_rewarded{};

void init();

bool is_ads_removed();

void handle_game_over();

void show_video(const show_video_callback& callback);

void purchase_remove_ads();

};

}