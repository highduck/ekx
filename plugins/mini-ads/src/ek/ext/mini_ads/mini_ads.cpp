#include "mini_ads.hpp"

namespace ek::mini_ads {

struct ads_context {
    show_video_callback video_callback;
    bool ads_removed = false;
};

static ads_context ctx;

void handle_game_over() {
    if (!ctx.ads_removed) {
        ads_show_interstitial();
    }
}

static void onAdsEvents(ads_event_type type) {
    switch (type) {
        case ads_event_type::video_rewarded:
            if (ctx.video_callback) {
                auto pr = ctx.video_callback;
                ctx.video_callback = {};
                pr(true);
            }
            on_video_rewarded();
            break;

        case ads_event_type::video_loading:
            break;

        case ads_event_type::video_failed:
            if (ctx.video_callback) {
                auto pr = ctx.video_callback;
                ctx.video_callback = {};
                pr(false);
            }
            break;
        case ads_event_type::video_closed:
            if (ctx.video_callback) {
                auto pr = ctx.video_callback;
                ctx.video_callback = {};
                pr(false);
            }
            break;
        case ads_event_type::video_loaded:
            if (ctx.video_callback) {
//                auto pr = sShowVideoCallback;
//                sShowVideoCallback = std::function<void(bool)>();
//                pr(true);
                ads_play_reward_video();
            }
            break;

        case ads_event_type::removed:
            ctx.ads_removed = true;
            on_removed();
            break;

        default:

            break;
    }
}

void show_video(const show_video_callback& callback) {
    ctx.video_callback = callback;
    ads_play_reward_video();
}

void purchase_remove_ads() {
    ads_purchase_remove();
}

bool is_ads_removed() {
    return ctx.ads_removed;
}

void init() {
    ads_listen(onAdsEvents);
}

}