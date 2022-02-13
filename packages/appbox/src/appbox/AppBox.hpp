#pragma once

#include <ek/ds/String.hpp>
#include <ecx/ecx.hpp>
#include <ek/admob.h>
#include <ek/util/Type.hpp>
#include "Ads.hpp"

namespace ek {

struct AppBoxConfig {
    const char* version_name = "1.0.0";
    const char* version_code = "";
    String privacyPolicyURL{"https://eliasku-games.web.app/privacy-policy/"};
    Ads::Config ads{};
    ek_admob_config admob{nullptr, nullptr, nullptr, EK_ADMOB_CHILD_DIRECTED_UNSPECIFIED};
    String billingKey{};

    // used for sharing results or app for link in the end of the message
    // TODO: could be better to share link object with text description
    String appLinkURL{};

    // currently only for manual "rate us" feature
    String appID{};

    struct DownloadAppURLs {
        String googlePlay;
        String appStore;
    };

    DownloadAppURLs downloadApp;
};

class AppBox {
public:
    explicit AppBox(AppBoxConfig info);

    void initDefaultControls(ecs::Entity e);

    void shareWithAppLink(const String& text);

    void rateUs() const;

    void initDownloadAppButtons(ecs::Entity e);

    void initLanguageButton(ecs::Entity e);

    void showAchievements();

public:
    AppBoxConfig config;

private:
};

class Achievement {
public:

    Achievement() = delete;

    explicit Achievement(const char* code, int count);

    void run() const;

private:
    const char* code_;
    int count_;
};

}

extern ek::AppBox* g_app_box;
void init_app_box(ek::AppBoxConfig config);

