#include "canvas_system.h"

#include <platform/Window.h>
#include <scenex/components/canvas.h>
#include <scenex/components/node_t.h>
#include <scenex/components/transform_2d.h>

namespace scenex {

inline ek::float2 get_screen_size() {
    auto size = ek::gWindow.backBufferSize;
    return {
            static_cast<float>(size.width),
            static_cast<float>(size.height)
    };
}

void on_scale_factor_changed() {
    // Sprite.setScale(scaleFactor);
}

void on_rect_changed() {
}

float2 get_canvas_space_size(ecs::entity e) {
    float2 size = get_screen_size();

    auto parent = ecs::get_or_default<node_t>(e).parent;
    if (parent) {
        auto transform = ecs::get<transform_2d>(parent);
        if (!transform.rect.empty()) {
            size = transform.rect.size;
        }
    }

    return size;
}

float update_canvas(ecs::entity e) {
    auto& canvas = ecs::get<canvas_t>(e);
    auto size = get_canvas_space_size(e);
    auto resolution_size = canvas.resolution.size;
    float2 scale_ratio = size / resolution_size;

    float scale = canvas.landscape ? scale_ratio.y : scale_ratio.x;
    float2 left_top = 0.5f * (size - scale * resolution_size);

    if (canvas.scale != scale) {
        canvas.scale = scale;
        on_scale_factor_changed();
    }

    rect_f rc{
            -left_top / scale,
            size / scale
    };

    auto& transform = ecs::get<transform_2d>(e);
    if (transform.rect != rc) {
        transform.matrix.position(left_top);
        transform.scale = float2{scale, scale};
        transform.rect = rc;
        on_rect_changed();
    }

    return scale;
}

void update_canvas() {
    for (auto e : ecs::view<canvas_t, transform_2d>()) {
        update_canvas(e);
    }
}

bool check_aspect_ratio(const ecs::entity e) {
    const auto& canvas = ecs::get<canvas_t>(e);
    const auto& resolution = canvas.resolution;
    const auto& view = ecs::get<transform_2d>(e).rect;
    const float resolution_ratio = resolution.width / resolution.height;
    const float view_ratio = view.width / view.height;
    return canvas.landscape ? view_ratio >= resolution_ratio : view_ratio <= resolution_ratio;
}

}
