#include "AppBox.hpp"

#include <ek/app.h>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <billing.hpp>
#include <ekx/app/audio_manager.h>
#include <utility>
#include <ek/game_services.h>
#include <ekx/app/localization.h>
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
    lang_name_t lang = {};
    int n = ek_ls_get_s("selected_lang", lang.str, sizeof(lang_name_t));
    EK_ASSERT(sizeof(lang_name_t) <= sizeof(ek_app.lang));
    if (n < 2) {
        memcpy(lang.str, ek_app.lang, sizeof(lang_name_t));
    }
    if (lang.str[0] == 0) {
        lang.str[0] = 'e';
        lang.str[1] = 'n';
    }
    // trim to 2-wide code
    lang.str[2] = 0;
    set_language(lang);
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
            if (g_ads->removed) {
                btn.get<Node>().setVisible(false);
            } else {
                g_ads->onRemoved << [btn] {
                    if (btn.isAlive()) {
                        btn.get<Node>().setVisible(false);
                    }
                };
                btn.get<Button>().clicked += [] {
                    g_ads->purchaseRemoveAds();
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
        {
            auto btn = find(e, H("sound"));
            if (btn) {
                btn.get<Button>().clicked += [btn] {
                    set_state_on_off(btn, audio_toggle_pref(AUDIO_PREF_SOUND));
                };
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_SOUND);
            }
        }
        {
            auto btn = find(e, H("music"));
            if (btn) {
                btn.get<Button>().clicked += [btn] {
                    set_state_on_off(btn, audio_toggle_pref(AUDIO_PREF_MUSIC));
                };
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_MUSIC);
            }
        }
        {
            auto btn = find(e, H("vibro"));
            if (btn) {
                btn.get<Button>().clicked += [btn] {
                    set_state_on_off(btn, audio_toggle_pref(AUDIO_PREF_VIBRO));
                    if (g_audio.prefs & AUDIO_PREF_VIBRO) {
                        vibrate(50);
                    }
                };
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_VIBRO);
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
            uint32_t index = s_localization.lang_index;
            uint32_t num = s_localization.lang_num;
            // check if langs are available
            if (index < num && num != 0 &&
                set_language_index((index + 1) % num)) {
                const char* lang_name = s_localization.languages[s_localization.lang_index].name.str;
                ek_ls_set_s("selected_lang", lang_name);
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

ek::AppBox* g_app_box = nullptr;

void init_app_box(ek::AppBoxConfig config) {
    EK_ASSERT(!g_app_box);
    g_app_box = new ek::AppBox(config);
}
