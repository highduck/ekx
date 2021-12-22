#pragma once

#include <ek/admob_wrapper.hpp>
#include <ek/scenex/app/GameAppListener.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/app.h>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/audio/audio.hpp>
#include <ek/time.hpp>

namespace ek {

class AdMobSimulator : public AdMobWrapper, public GameAppListener {

    bool activeRewardedAd = false;

public:

    void onRenderFrame() override {
        const bool splash = activeInterstitial || activeRewardedAd;
        const argb32_t color = activeRewardedAd ? 0x77FF00FF_argb : 0x7700FF00_argb;

        if (splash) {
            draw2d::state.setEmptyTexture();
            draw2d::quad(0, 0, ek_app.viewport.width, ek_app.viewport.height, color);
        }
    }

    AdMobSimulator() {
        Locator::ref<basic_application>().dispatcher.listeners.push_back(this);
    }

    ~AdMobSimulator() override {
        Locator::ref<basic_application>().dispatcher.listeners.remove(this);
    }

    void showInterstitial(std::function<void()> callback) override {
        activeInterstitial = true;
        ek_audio_mute_push();
        ek_set_timeout(ek::timer_func([this, cb = std::move(callback)] {
            activeInterstitial = false;
            ek_audio_mute_pop();
            if (cb) {
                cb();
            }
        }), 3);
    }

    void showRewardedAd(std::function<void(bool)> callback) override {
        activeRewardedAd = true;
        ek_audio_mute_push();
        ek_set_timeout(ek::timer_func([this, cb = std::move(callback)] {
            activeRewardedAd = false;
            ek_audio_mute_pop();
            if (cb) {
                cb(true);
            }
        }), 5);
    }
};

}

