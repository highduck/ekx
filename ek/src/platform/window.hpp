#pragma once

#include <cstdint>
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

enum class mouse_cursor_t : uint8_t {
    parent = 0,
    arrow,
    button,
    help
};

typedef void* native_window_context_t;

class window_t final {
public:

    struct dimensions_t final {
        uint32_t width = 1;
        uint32_t height = 1;
    };

    dimensions_t window_size{};
    dimensions_t back_buffer_size{};
    float device_pixel_ratio = 1.0f;

    // iOS has manually created FBO for primary surface
    // so it will be set up there
    uint32_t primary_frame_buffer = 0;

    [[nodiscard]]
    native_window_context_t view_context() const;

    [[nodiscard]]
    mouse_cursor_t cursor() const {
        return cursor_;
    }

    void set_cursor(mouse_cursor_t cursor) {
        cursor_ = cursor;
        update_mouse_cursor();
    }

    bool size_changed = false;

    window_creation_config_t creation_config;

private:

    void update_mouse_cursor();

    mouse_cursor_t cursor_ = mouse_cursor_t::parent;

};

extern window_t g_window;

}


