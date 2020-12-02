#pragma once

#include <string>
#include <ecxx/ecxx.hpp>
#include <admob.hpp>
#include "Ads.hpp"

namespace ek {

struct AppBoxConfig {
    std::string version{"1.0.0"};
    std::string privacyPolicyURL{"https://eliasku.gitlab.io/privacy-policy/"};
    Ads::Config ads{};
    ::admob::config_t admob{};
    std::string billingKey{};

    // used for sharing results or app for link in the end of the message
    // TODO: could be better to share link object with text description
    std::string appLinkURL{};

    // currently only for manual "rate us" feature
    std::string appID{};

    struct DownloadAppURLs {
        std::string googlePlay;
        std::string appStore;
    };

    DownloadAppURLs downloadApp;
};

class AppBox {
public:
    explicit AppBox(AppBoxConfig info);

    void initDefaultControls(ecs::entity e);

    void shareWithAppLink(const std::string& text);

    void rateUs();


    void initDownloadAppButtons(ecs::entity e);

public:
    AppBoxConfig config;

private:
};



}

