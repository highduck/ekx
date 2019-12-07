#include "window.hpp"

namespace ek {

window_creation_config_t::window_creation_config_t(std::string title_, float width_, float height_) noexcept
        : title{std::move(title_)},
          width{width_},
          height{height_},
          landscape{width > height},
          fullscreen{false} {
}

window_creation_config_t::window_creation_config_t() noexcept
        : title{},
          width{960.0f},
          height{720.0f},
          landscape{true},
          fullscreen{false} {

}

window_t g_window{};

}