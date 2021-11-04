#include "Viewport.hpp"
#include "LayoutRect.hpp"
#include "../app/GameDisplay.hpp"

namespace ek {

void doScale(const ViewportScaleInput& input, const ViewportScaleOptions& options, ViewportScaleOutput& output) {
    auto fullRect = input.fullRect;
    fullRect.position += options.viewport.position * fullRect.size;
    fullRect.size = fullRect.size * options.viewport.size;
    const auto safeRect = clamp_bounds(input.safeRect, fullRect);

    if (options.scaleToResolution) {
        // interpolate between full and safe area
        const rect_f rc{
                fullRect.position + (safeRect.position - fullRect.position) * options.safeAreaFit,
                fullRect.size + (safeRect.size - fullRect.size) * options.safeAreaFit
        };
        const float width = rc.width;
        const float height = rc.height;
        const float scale = fminf(width / options.baseResolution.x, height / options.baseResolution.y);
        output.offset = options.alignment * (rc.size - scale * options.baseResolution);
        output.scale = scale;
    } else {
        output.scale = length(options.pixelRatio);
        const rect_f rc{
                fullRect.position + (safeRect.position - fullRect.position) * options.safeAreaFit,
                fullRect.size + (safeRect.size - fullRect.size) * options.safeAreaFit
        };
        output.offset = options.alignment * (rc.size - output.scale * options.baseResolution);
    }

    output.offset += safeRect.position * options.safeAreaFit;

    const auto invScale = 1.0f / output.scale;
    output.fullRect = translate(fullRect, -output.offset) * invScale;
    output.safeRect = translate(safeRect, -output.offset) * invScale;

    output.screenRect.position = input.fullRect.position + options.viewport.position * input.fullRect.size;
    output.screenRect.size = options.viewport.size * input.fullRect.size;
}

void updateViewport(ecs::EntityApi e, const ViewportScaleInput& input) {
    auto& vp = e.get<Viewport>();

    doScale(input, vp.options, vp.output);

    auto& layout = e.get<LayoutRect>();
    layout.rect = vp.output.fullRect;
    layout.safeRect = vp.output.safeRect;
}

void Viewport::updateAll(const GameDisplayInfo& display0) {
    const float w = display0.size.x;
    const float h = display0.size.y;
    const float4 insets = display0.insets + display0.userInsetsAbsolute + float4{w, h, w, h} * display0.userInsetsRelative;
    ViewportScaleInput input;
    input.fullRect = {0.0f, 0.0f, display0.size.x, display0.size.y};
    input.safeRect = {insets.x, insets.y,w - insets.x - insets.z,h - insets.y - insets.w};
    input.dpiScale = display0.dpiScale;

    for (auto e : ecs::view<Viewport>()) {
        updateViewport(e, input);
    }
}

}
