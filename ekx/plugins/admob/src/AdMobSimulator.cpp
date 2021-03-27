#include "AdMobSimulator.hpp"

#include <ek/draw2d/drawer.hpp>
#include <ek/app/app.hpp>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/timers.hpp>
#include <ek/audio/audio.hpp>

namespace ek {

void AdMobSimulator::draw() const {
    if (activeInterstitial) {
        auto size = app::g_app.drawable_size;
        draw2d::state.set_empty_texture();
        draw2d::quad(0, 0,
                     static_cast<float>(size.x),
                     static_cast<float>(size.y), 0x7700FF00_argb);
    }

    if (activeRewardedAd) {
        auto size = app::g_app.drawable_size;
        draw2d::state.set_empty_texture();
        draw2d::quad(0, 0,
                     static_cast<float>(size.x),
                     static_cast<float>(size.y), 0x77FF00FF_argb);
    }
}

AdMobSimulator::AdMobSimulator() {
    resolve<basic_application>().hook_on_render_frame +=
            [this] { this->draw(); };
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
}