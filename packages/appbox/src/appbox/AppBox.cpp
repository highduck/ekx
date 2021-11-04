#include "AppBox.hpp"

#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <billing.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/util/ServiceLocator.hpp>
#include <ek/util/StringUtil.hpp>
#include <utility>
#include <GameServices.hpp>
#include <ek/ext/sharing/sharing.hpp>
#include <ek/goodies/GameScreen.hpp>
#include <ek/app/app.hpp>
#include <ek/Localization.hpp>
#include "Ads.hpp"

namespace ek {

AppBox::AppBox(AppBoxConfig config_) :
        config{std::move(config_)} {

    // unlock abort()

    billing::initialize(config.billingKey);
    admob::initialize(config.admob);
    Locator::create<Ads>(config.ads);
    game_services_init();

    // initialize translations
    auto lang = get_user_string("selected_lang", "");
    if (lang.empty()) {
        const char* l = app::getPreferredLang();
        lang = l != nullptr ? l : "en";
    }
    if (lang.size() > 2) {
        lang.resize(2);
    }
    Localization::instance.setLanguage(lang);
}

void set_state_by_name(ecs::EntityApi e, const std::string &state) {
    eachChild(e, [&state](ecs::EntityApi child) {
        const auto &name = child.get_or_default<NodeName>().name;
        if (starts_with(name, "state_")) {
            setVisible(child, name == state);
        }
    });
}

void set_state_on_off(ecs::EntityApi e, bool enabled) {
    set_state_by_name(e, enabled ? "state_on" : "state_off");
}

void AppBox::initDefaultControls(ecs::EntityApi e) {
    {
        // VERSION
        auto e_version = find(e, "version");
        if (e_version) {
            auto *txt = Display2D::tryGet<Text2D>(e_version);
            if (txt) {
                txt->text = config.version;
            }
        }
    }
    {
        // PRIVACY POLICY
        auto e_pp = find(e, "privacy_policy");
        if (e_pp) {
            auto lbl = find(e_pp, "label");
            if (lbl) {
                auto *txt = Display2D::tryGet<Text2D>(lbl);
                if(txt) {
                    txt->hitFullBounds = true;
                }
            }
            e_pp.get_or_create<Interactive>();
            e_pp.get_or_create<Button>().clicked += [this] {
                app::openURL(config.privacyPolicyURL.c_str());
            };
        }
    }

    // Purchases
    {
        auto btn = find(e, "remove_ads");
        if (btn) {
            auto &ads = Locator::ref<Ads>();
            if (ads.isRemoved()) {
                btn.get<Node>().setVisible(false);
            } else {
                ads.onRemoved << [btn] {
                    if (btn.isAlive()) {
                        btn.get<Node>().setVisible(false);
                    }
                };
                btn.get<Button>().clicked += [] {
                    Locator::ref<Ads>().purchaseRemoveAds();
                };
            }
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
        auto &audio = Locator::ref<AudioManager>();
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

        initLanguageButton(e);
    }
}

void AppBox::shareWithAppLink(const std::string &text) {
    auto msg = text;
    if (!config.appLinkURL.empty()) {
        msg += ' ';
        msg += config.appLinkURL;
    }
    sharing_send_message(msg.c_str());
}

void AppBox::rateUs() {
    sharing_rate_us(config.appID.c_str());
}

/// download app feature

void wrap_button(ecs::EntityApi e, const std::string &name, const std::string &link) {
    auto x = find(e, name);
    if (!link.empty()) {
        x.get_or_create<Button>().clicked.add([link] {
            app::openURL(link.c_str());
        });
    } else {
        setVisible(e, false);
    }
}

void AppBox::initDownloadAppButtons(ecs::EntityApi) {
//    auto banner = sg_create("gfx", "cross_banner");
//    setName(banner, "banner");
//    layout_wrapper{banner}.aligned(0.5f, 0.0f, 1.0f, 0.0f);
//
//    wrap_button(banner, "google_play", config.downloadApp.googlePlay);
//    wrap_button(banner, "app_store", config.downloadApp.appStore);
//
//    append(e, banner);
}

void AppBox::initLanguageButton(ecs::EntityApi e) {
    auto btn = find(e, "language");
    if (btn) {
        btn.get<Button>().clicked += [] {
            auto &lm = Localization::instance;
            auto &locales = lm.getAvailableLanguages();
            auto locale = std::find(locales.begin(), locales.end(), lm.getLanguage());
            if (locale != locales.end()) {
                ++locale;
                if (locale == locales.end()) {
                    locale = locales.begin();
                }
                auto &lang = *locale;
                lm.setLanguage(lang);
                set_user_string("selected_lang", lang.c_str());
            }
        };
    }
}

void AppBox::showAchievements() {
    achievement_show();
}

Leaderboard::Leaderboard(const char *id) :
        id_{id} {

}

void Leaderboard::show() const {
    leader_board_show(id_.c_str());
}

void Leaderboard::submit(int score) const {
    leader_board_submit(id_.c_str(), score);
}

Achievement::Achievement(const char *code, int count) :
        code_{code},
        count_{count} {
}

void Achievement::run() const {
    achievement_update(code_.c_str(), count_);
}

}