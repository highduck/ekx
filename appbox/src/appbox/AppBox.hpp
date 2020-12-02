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
};

class AppBox {
public:
    explicit AppBox(AppBoxConfig info);

    void initDefaultControls(ecs::entity e);

public:
    AppBoxConfig config;

private:
};



}

