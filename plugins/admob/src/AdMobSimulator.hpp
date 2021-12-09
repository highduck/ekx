#pragma once

#include "AdMobWrapper.hpp"
#include <ek/scenex/app/GameAppListener.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/app/app.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/timers.hpp>
#include <ek/audio/audio.hpp>

namespace ek {

class AdMobSimulator : public AdMobWrapper, public GameAppListener {

    bool activeRewardedAd = false;

public:

    void onRenderFrame() override {
        using ek::app::g_app;

        const bool splash = activeInterstitial || activeRewardedAd;
        const argb32_t color = activeRewardedAd ? 0x77FF00FF_argb : 0x7700FF00_argb;

        if (splash) {
            draw2d::state.setEmptyTexture();
            draw2d::quad(0, 0, g_app.drawableWidth, g_app.drawableHeight, color);
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
        audio::muteDeviceBegin();
        setTimeout([this, cb=std::move(callback)] {
            activeInterstitial = false;
            audio::muteDeviceEnd();
            if (cb) {
                cb();
            }
        }, 3.0f);
    }

    void showRewardedAd(std::function<void(bool)> callback) override {
        activeRewardedAd = true;
        audio::muteDeviceBegin();
        setTimeout([this, cb = std::move(callback)] {
            activeRewardedAd = false;
            audio::muteDeviceEnd();
            if (cb) {
                cb(true);
            }
        }, 5.0f);
    }
};

}

