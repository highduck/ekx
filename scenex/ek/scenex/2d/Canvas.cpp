#include "Canvas.hpp"
#include "LayoutRect.hpp"
#include "Transform2D.hpp"

#include <ek/app/app.hpp>
#include <ek/scenex/base/Node.hpp>

using ek::app::g_app;

namespace ek {

inline float2 get_screen_size() {
    return {
            static_cast<float>(g_app.drawable_size.x),
            static_cast<float>(g_app.drawable_size.y)
    };
}

LayoutRect get_canvas_space_size(ecs::entity e) {
    LayoutRect result;
    result.rect = {float2::zero, get_screen_size()};
    result.safeRect = result.rect;

    auto parent = e.get_or_default<Node>().parent;
    if (parent) {
        auto layout = parent.get<LayoutRect>();
        if (!layout.safeRect.empty()) {
            result = layout;
        }
    }

    return result;
}

struct CanvasSystemInput {
    // screen metrics
    rect_f fullRect;
    rect_f safeRect;
    // logical resolution size
    float2 resolution;
    // modes
    bool landscape;
};

struct CanvasSystemOutput {
    rect_f fullRect;
    rect_f safeRect;
    float2 offset;
    float scale;
};

void calculateCanvas(CanvasSystemInput input, CanvasSystemOutput& output) {
    float2 scale_ratio = input.safeRect.size / input.resolution;

    const float aspect = input.fullRect.width / input.fullRect.height;
    const float baseAspect = input.resolution.x / input.resolution.y;
    if (!input.landscape && baseAspect < aspect) {
        scale_ratio *= baseAspect / aspect;
    } else if (input.landscape && baseAspect > aspect) {
        scale_ratio *= aspect / baseAspect;
    }

    const float scale = input.landscape ? scale_ratio.y : scale_ratio.x;
    const float2 left_top = 0.5f * (input.safeRect.size - scale * input.resolution);
    const float2 left_top_full = 0.5f * (input.fullRect.size - scale * input.resolution);
    const auto fullRectPosition = (input.fullRect.position - left_top_full) / scale;

    output.fullRect.position = fullRectPosition;
    output.fullRect.size = input.fullRect.size / scale;
    output.safeRect.position = fullRectPosition + input.safeRect.position / scale;
    output.safeRect.size = input.safeRect.size / scale;
    output.offset = left_top_full;
    output.scale = scale;
}

void update_canvas(ecs::entity e) {
    auto& canvas = e.get<Canvas>();
    auto rootLayout = get_canvas_space_size(e);
    CanvasSystemInput input;
    input.resolution = canvas.resolution;
    input.fullRect = rootLayout.rect;
    input.safeRect = rootLayout.safeRect;
    input.landscape = canvas.landscape;
    CanvasSystemOutput result;
    calculateCanvas(input, result);
    canvas.scale = result.scale;
    auto& layout = e.get<LayoutRect>();
    if (layout.safeRect != result.safeRect) {
        layout.safeRect = result.safeRect;
        layout.rect = result.fullRect;

        auto& transform = e.get<Transform2D>();
        transform.setPosition(result.offset);
        transform.setScale(result.scale);
    }
}

void Canvas::updateAll() {
    for (auto e : ecs::view<Canvas>()) {
        update_canvas(e);
    }
}

}
