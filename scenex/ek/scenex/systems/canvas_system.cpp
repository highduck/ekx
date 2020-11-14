#include "canvas_system.hpp"
#include "layout_system.hpp"

#include <ek/app/app.hpp>
#include <ek/scenex/components/canvas.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/components/transform_2d.hpp>

using ek::app::g_app;

namespace ek {

inline float2 get_screen_size() {
    return {
            static_cast<float>(g_app.drawable_size.x),
            static_cast<float>(g_app.drawable_size.y)
    };
}

void on_scale_factor_changed() {
}

void on_rect_changed() {
}

layout_t get_canvas_space_size(ecs::entity e) {
    layout_t result;
    result.rect = {float2::zero, get_screen_size()};
    result.safeRect = result.rect;

    auto parent = ecs::get_or_default<node_t>(e).parent;
    if (parent) {
        auto layout = ecs::get<layout_t>(parent);
        if (!layout.safeRect.empty()) {
            result = layout;
        }
    }

    return result;
}

float update_canvas(ecs::entity e) {
    auto& canvas = ecs::get<canvas_t>(e);
    auto rootLayout = get_canvas_space_size(e);
    auto resolution_size = canvas.resolution.size;
    float2 scale_ratio = rootLayout.safeRect.size / resolution_size;

    float aspect = rootLayout.rect.width / rootLayout.rect.height;
    float baseAspect = resolution_size.x / resolution_size.y;
    if (!canvas.landscape && baseAspect < aspect) {
        scale_ratio *= baseAspect / aspect;
    } else if (canvas.landscape && baseAspect > aspect) {
        scale_ratio *= aspect / baseAspect;
    }

    float scale = canvas.landscape ? scale_ratio.y : scale_ratio.x;
    float2 left_top = 0.5f * (rootLayout.safeRect.size - scale * resolution_size);
    float2 left_top_full = 0.5f * (rootLayout.rect.size - scale * resolution_size);

    if (canvas.scale != scale) {
        canvas.scale = scale;
        on_scale_factor_changed();
    }

    rect_f rect{
            (rootLayout.rect.position - left_top_full) / scale,
            rootLayout.rect.size / scale
    };

    rect_f safeRect{
            rect.position + rootLayout.safeRect.position / scale,
            rootLayout.safeRect.size / scale
    };

    auto& layout = ecs::get<layout_t>(e);
    if (layout.safeRect != safeRect) {
        layout.safeRect = safeRect;
        layout.rect = rect;

        auto& transform = ecs::get<transform_2d>(e);
        transform.position = left_top_full;
        transform.scale = float2{scale, scale};

        on_rect_changed();
    }

    return scale;
}

void update_canvas() {
    for (auto e : ecs::view<canvas_t, transform_2d>()) {
        update_canvas(e);
    }
}

//bool check_aspect_ratio(const ecs::entity e) {
//    const auto& canvas = ecs::get<canvas_t>(e);
//    const auto& resolution = canvas.resolution;
//    const auto& view = ecs::get<transform_2d>(e).rect;
//    const float resolution_ratio = resolution.width / resolution.height;
//    const float view_ratio = view.width / view.height;
//    return canvas.landscape ? view_ratio >= resolution_ratio : view_ratio <= resolution_ratio;
//}

}
