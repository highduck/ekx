#pragma once

#include <ek/ds/String.hpp>
#include <ecxx/ecxx.hpp>
#include <admob.hpp>
#include <ek/util/Type.hpp>
#include "Ads.hpp"

namespace ek {

struct AppBoxConfig {
    String version{"1.0.0"};
    String privacyPolicyURL{"https://eliasku-games.web.app/privacy-policy/"};
    Ads::Config ads{};
    ::admob::Config admob{};
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

    void initDefaultControls(ecs::EntityApi e);

    void shareWithAppLink(const String& text);

    void rateUs();

    void initDownloadAppButtons(ecs::EntityApi e);

    void initLanguageButton(ecs::EntityApi e);

    void showAchievements();

public:
    AppBoxConfig config;

private:
};

EK_DECLARE_TYPE(AppBox);

class Leaderboard {
public:
    Leaderboard() = delete;

    explicit Leaderboard(const char* id);

    void show() const;

    void submit(int score) const;

private:
    String id_;
};

class Achievement {
public:

    Achievement() = delete;

    explicit Achievement(const char* code, int count);

    void run() const;

private:
    String code_;
    int count_;
};

}

