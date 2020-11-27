#pragma once

#include <string>
#include <ecxx/ecxx.hpp>

namespace ek {

struct AppInfo {
    std::string version{"1.0.0"};
    std::string privacyPolicyURL{"https://eliasku.gitlab.io/privacy-policy/"};

};

class StandardFeatures {
public:
    explicit StandardFeatures(AppInfo info);

    AppInfo info;

    void initDefaultControls(ecs::entity e);
};



}

