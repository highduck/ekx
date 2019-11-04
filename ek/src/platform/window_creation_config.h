#pragma once

#include <string>

namespace ek {

struct window_creation_config_t {

    std::string title;
    float width;
    float height;
    bool landscape;
    bool fullscreen;

    window_creation_config_t() noexcept;

    window_creation_config_t(std::string title, float width, float height) noexcept;
};

}

