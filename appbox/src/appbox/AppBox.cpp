#include "AppBox.hpp"

#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <billing.hpp>
#include <ek/scenex/utility/basic_game_utility.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/util/locator.hpp>
#include <ek/util/strings.hpp>
#include <utility>
#include <ek/ext/game_center/game_center.hpp>
#include "Ads.hpp"

namespace ek {

AppBox::AppBox(AppBoxConfig config_) :
        config{std::move(config_)} {

    billing::initialize(config.billingKey);
    admob::initialize(config.admob);
    service_locator_instance<Ads>::init(config.ads);
    game_services_init();
}

void set_state_by_name(ecs::entity e, const std::string& state) {
    eachChild(e, [&state](ecs::entity child) {
        const auto& name = ecs::get_or_default<Node>(child).name;
        if (starts_with(name, "state_")) {
            setVisible(child, name == state);
        }
    });
}

void set_state_on_off(ecs::entity e, bool enabled) {
    set_state_by_name(e, enabled ? "state_on" : "state_off");
}

void AppBox::initDefaultControls(ecs::entity e) {
    {
        // VERSION
        auto e_version = find(e, "version");
        if (e_version) {
            e_version.get<Display2D>().get<Text2D>()->text = "VERSION: " + config.version;
        }
    }
    {
        // PRIVACY POLICY
        auto e_pp = find(e, "privacy_policy");
        if (e_pp) {
            auto lbl = find(e_pp, "label");
            if (lbl) {
                auto* txt = lbl.get<Display2D>().get<Text2D>();
                txt->hitFullBounds = true;
                txt->text = "PRIVACY POLICY";
            }
            e_pp.get_or_create<Interactive>();
            e_pp.get_or_create<Button>().clicked += [this] {
                BasicGameUtility::navigate(config.privacyPolicyURL, true);
            };
        }
    }

    // Purchases
    {
        auto btn = find(e, "remove_ads");
        if (btn) {
            btn.get<Node>().setVisible(!resolve<Ads>().isRemoved());
            btn.get<Button>().clicked += [] {
                resolve<Ads>().purchaseRemoveAds();
            };
        }
    }
    {
        auto btn = find(e, "restore_purchases");
        if (btn) {
            btn.get<Button>().clicked += [] {
                billing::getPurchases();
            };
        }
    }

    // Settings
    {
        auto& audio = resolve<AudioManager>();
        {
            auto btn = find(e, "sound");
            if (btn) {
                btn.get<Button>().clicked += [btn, &audio] {
                    set_state_on_off(btn, audio.sound.toggle());
                };
                set_state_on_off(btn, audio.sound.enabled());
            }
        }
        {
            auto btn = find(e, "music");
            if (btn) {
                btn.get<Button>().clicked += [btn, &audio] {
                    set_state_on_off(btn, audio.music.toggle());
                };
                set_state_on_off(btn, audio.music.enabled());
            }
        }
        {
            auto btn = find(e, "vibro");
            if (btn) {
                btn.get<Button>().clicked += [btn, &audio] {
                    set_state_on_off(btn, audio.vibro.toggle());
                    if (audio.vibro.enabled()) {
                        audio.vibrate(50);
                    }
                };
                set_state_on_off(btn, audio.vibro.enabled());
            }
        }
    }
}

}