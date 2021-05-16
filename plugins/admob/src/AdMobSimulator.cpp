#include "AdMobSimulator.hpp"

#include <ek/draw2d/drawer.hpp>
#include <ek/app/app.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/timers.hpp>
#include <ek/audio/audio.hpp>

namespace ek {

AdMobSimulator::AdMobSimulator() {
    Locator::ref<basic_application>().dispatcher.listeners.push_back(this);
}

void AdMobSimulator::showInterstitial(const std::function<void()>& callback) {
    activeInterstitial = true;
    audio::muteDeviceBegin();
    setTimeout([this, callback] {
        activeInterstitial = false;
        audio::muteDeviceEnd();
        if (callback) {
            callback();
        }
    }, 3.0f);
}

void AdMobSimulator::showRewardedAd(const std::function<void(bool)>& callback) {
    activeRewardedAd = true;
    audio::muteDeviceBegin();
    setTimeout([this, callback] {
        activeRewardedAd = false;
        audio::muteDeviceEnd();
        if (callback) {
            callback(true);
        }
    }, 5.0f);
}

void AdMobSimulator::onRenderFrame() {
    if (activeInterstitial) {
        auto size = app::g_app.drawable_size;
        draw2d::state.setEmptyTexture();
        draw2d::quad(0, 0,
                     static_cast<float>(size.x),
                     static_cast<float>(size.y), 0x7700FF00_argb);
    }

    if (activeRewardedAd) {
        auto size = app::g_app.drawable_size;
        draw2d::state.setEmptyTexture();
        draw2d::quad(0, 0,
                     static_cast<float>(size.x),
                     static_cast<float>(size.y), 0x77FF00FF_argb);
    }
}

AdMobSimulator::~AdMobSimulator() {
    Locator::ref<basic_application>().dispatcher.listeners.remove(this);
}

}