#include "AppBox.hpp"

#include <ek/app.h>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <billing.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/util/ServiceLocator.hpp>
#include <utility>
#include <ek/game_services.h>
#include <ek/goodies/GameScreen.hpp>
#include <ek/scenex/Localization.hpp>
#include "Ads.hpp"

namespace ek {

AppBox::AppBox(AppBoxConfig config_) :
        config{std::move(config_)} {

    // unlock abort()

    billing::initialize(config.billingKey.c_str());
    ek_admob_init(config.admob);
    ads_init(config.ads);
    ek_game_services_init();

    // initialize translations
    // TODO: wtf
    char lang_buf[sizeof(ek_app.lang)];
    int n = ek_ls_get_s("selected_lang", lang_buf, sizeof(lang_buf));
    if (n < 2) {
        memcpy(lang_buf, ek_app.lang, sizeof(lang_buf));
    }
    if (lang_buf[0] == 0) {
        lang_buf[0] = 'e';
        lang_buf[1] = 'n';
    }
    lang_buf[2] = 0;
    Localization::instance.setLanguage(lang_buf);
}

void set_state_on_off(ecs::EntityApi e, bool enabled) {
    auto on = find(e, H("state_on"));
    auto off = find(e, H("state_off"));
    setVisible(on, enabled);
    setVisible(off, !enabled);
}

void AppBox::initDefaultControls(ecs::EntityApi e) {
    {
        // VERSION
        auto e_version = find(e, H("version"));
        if (e_version) {
            auto* txt = Display2D::tryGet<Text2D>(e_version);
            if (txt) {
                txt->text = config.version;
            }
        }
    }
    {
        // PRIVACY POLICY
        auto e_pp = find(e, H("privacy_policy"));
        if (e_pp) {
            auto lbl = find(e_pp, H("label"));
            if (lbl) {
                auto* txt = Display2D::tryGet<Text2D>(lbl);
                if (txt) {
                    txt->hitFullBounds = true;
                }
            }
            e_pp.get_or_create<Interactive>();
            e_pp.get_or_create<Button>().clicked += [this] {
                ek_app_open_url(config.privacyPolicyURL.c_str());
            };
        }
    }

    // Purchases
    {
        auto btn = find(e, H("remove_ads"));
        if (btn) {
            auto& ads = Locator::ref<Ads>();
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
        auto btn = find(e, H("restore_purchases"));
        if (btn) {
            btn.get<Button>().clicked += [] {
                billing::getPurchases();
            };
        }
    }

    // Settings
    {
        auto& audio = Locator::ref<AudioManager>();
        {
            auto btn = find(e, H("sound"));
            if (btn) {
                btn.get<Button>().clicked += [btn, &audio] {
                    set_state_on_off(btn, audio.sound.toggle());
                };
                set_state_on_off(btn, audio.sound.enabled());
            }
        }
        {
            auto btn = find(e, H("music"));
            if (btn) {
                btn.get<Button>().clicked += [btn, &audio] {
                    set_state_on_off(btn, audio.music.toggle());
                };
                set_state_on_off(btn, audio.music.enabled());
            }
        }
        {
            auto btn = find(e, H("vibro"));
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

void AppBox::shareWithAppLink(const String& text) {
    auto msg = text;
    if (!config.appLinkURL.empty()) {
        msg += ' ';
        msg += config.appLinkURL;
    }
    ek_app_share(msg.c_str());
}

void AppBox::rateUs() const {
#ifdef __ANDROID__
    char buf[1024];
    ek_snprintf(buf, 1024, "market://details?id=%s", config.appID.c_str());
    ek_app_open_url(buf);
#endif // __ANDROID__

#ifdef __APPLE__
    char buf[1024];
    ek_snprintf(buf, 1024, "itms-apps://itunes.apple.com/us/app/apple-store/id%s?mt=8&action=write-review",
                config.appID.c_str());
    ek_app_open_url(buf);
#endif // __APPLE__
}

/// download app feature

void wrap_button(ecs::EntityApi e, string_hash_t tag, const char* link) {
    auto x = find(e, tag);
    if (link && *link) {
        x.get_or_create<Button>().clicked.add([link] {
            ek_app_open_url(link);
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
    auto btn = find(e, H("language"));
    if (btn) {
        btn.get<Button>().clicked += [] {
            auto& lm = Localization::instance;
            auto& locales = lm.getAvailableLanguages();
            String* locale = locales.find(lm.getLanguage());
            if (locale) {
                ++locale;
                if (locale == locales.end()) {
                    locale = locales.begin();
                }
                auto& lang = *locale;
                lm.setLanguage(lang.c_str());
                ek_ls_set_s("selected_lang", lang.c_str());
            }
        };
    }
}

void AppBox::showAchievements() {
    ek_achievement_show();
}

Leaderboard::Leaderboard(const char* id) :
        id_{id} {

}

void Leaderboard::show() const {
    ek_leaderboard_show(id_.c_str());
}

void Leaderboard::submit(int score) const {
    ek_leaderboard_submit(id_.c_str(), score);
}

Achievement::Achievement(const char* code, int count) :
        code_{code},
        count_{count} {
}

void Achievement::run() const {
    ek_achievement_update(code_.c_str(), count_);
}

}