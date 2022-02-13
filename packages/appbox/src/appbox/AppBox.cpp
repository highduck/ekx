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
#include "ek/scenex/base/NodeEvents.hpp"

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

void set_state_on_off(ecs::Entity e, bool enabled) {
    auto on = find(e, H("state_on"));
    auto off = find(e, H("state_off"));
    set_visible(on, enabled);
    set_visible(off, !enabled);
}

void AppBox::initDefaultControls(ecs::Entity e) {
    {
        // VERSION
        auto e_version = find(e, H("version"));
        if (e_version.id) {
            set_text_f(e_version, "%s #%s", config.version_name, config.version_code);
        }
    }
    {
        // PRIVACY POLICY
        ecs::Entity e_pp = find(e, H("privacy_policy"));
        if (e_pp) {
            auto lbl = find(e_pp, H("label"));
            if (lbl.id) {
                auto* txt = ecs::try_get<Text2D>(lbl);
                if (txt) {
                    txt->hitFullBounds = true;
                }
            }
            ecs::add<Interactive>(e_pp);
            ecs::add<Button>(e_pp);
            ecs::add<NodeEventHandler>(e_pp).on(BUTTON_EVENT_CLICK, [](const NodeEventData& ) {
                ek_app_open_url(g_app_box->config.privacyPolicyURL.c_str());
            });
        }
    }

    // Purchases
    {
        entity_t btn = find(e, H("remove_ads"));
        if (btn.id) {
            if (g_ads->removed) {
                set_visible(btn, false);
            } else {
                g_ads->onRemoved << [btn] {
                    if (is_entity(btn)) {
                        set_visible(btn, false);
                    }
                };
                ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& ) {
                    g_ads->purchaseRemoveAds();
                });
            }
        }
    }
    {
        entity_t btn = find(e, H("restore_purchases"));
        if (btn.id) {
            ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& ) {
                billing::getPurchases();
            });
        }
    }

    // Settings
    {
        {
            ecs::Entity btn = find(e, H("sound"));
            if (btn) {
                ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& event) {
                    set_state_on_off(ecs::Entity{event.source}, audio_toggle_pref(AUDIO_PREF_SOUND));
                });
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_SOUND);
            }
        }
        {
            ecs::Entity btn = find(e, H("music"));
            if (btn) {
                ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& event) {
                    set_state_on_off(ecs::Entity{event.source}, audio_toggle_pref(AUDIO_PREF_MUSIC));
                });
                set_state_on_off(btn, g_audio.prefs & AUDIO_PREF_MUSIC);
            }
        }
        {
            ecs::Entity btn = find(e, H("vibro"));
            if (btn) {
                ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& event) {
                    set_state_on_off(ecs::Entity{event.source}, audio_toggle_pref(AUDIO_PREF_VIBRO));
                    if (g_audio.prefs & AUDIO_PREF_VIBRO) {
                        vibrate(50);
                    }
                });
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

void wrap_button(ecs::Entity e, string_hash_t tag, const char* link) {
    ecs::Entity x = find(e, tag);
    if (link && *link) {
        ecs::add<Button>(x);
        ecs::add<NodeEventHandler>(x).on(BUTTON_EVENT_CLICK, [link](const NodeEventData& ) {
            ek_app_open_url(link);
        });
    } else {
        set_visible(e, false);
    }
}

void AppBox::initDownloadAppButtons(ecs::Entity) {
//    auto banner = sg_create("gfx", "cross_banner");
//    setName(banner, "banner");
//    layout_wrapper{banner}.aligned(0.5f, 0.0f, 1.0f, 0.0f);
//
//    wrap_button(banner, "google_play", config.downloadApp.googlePlay);
//    wrap_button(banner, "app_store", config.downloadApp.appStore);
//
//    append(e, banner);
}

void AppBox::initLanguageButton(ecs::Entity e) {
    ecs::Entity btn = find(e, H("language"));
    if (btn) {
        ecs::add<NodeEventHandler>(btn).on(BUTTON_EVENT_CLICK, [](const NodeEventData& ) {
            uint32_t index = s_localization.lang_index;
            uint32_t num = s_localization.lang_num;
            // check if langs are available
            if (index < num && num != 0 &&
                set_language_index((index + 1) % num)) {
                const char* lang_name = s_localization.languages[s_localization.lang_index].name.str;
                ek_ls_set_s("selected_lang", lang_name);
            }
        });
    }
}

void AppBox::showAchievements() {
    ek_achievement_show();
}

Achievement::Achievement(const char* code, int count) :
        code_{code},
        count_{count} {
}

void Achievement::run() const {
    ek_achievement_update(code_, count_);
}

}

ek::AppBox* g_app_box = nullptr;

void init_app_box(ek::AppBoxConfig config) {
    EK_ASSERT(!g_app_box);
    g_app_box = new ek::AppBox(config);
}
