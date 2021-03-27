#include "Viewport.hpp"
#include "LayoutRect.hpp"

namespace ek {

void doScale(ViewportScaleInput input, ViewportScaleOptions options, ViewportScaleOutput& output) {
    if (options.scaleToResolution) {
        // interpolate between full and safe area
        const rect_f rc{
                input.fullRect.position + (input.safeRect.position - input.fullRect.position) * options.safeAreaFit,
                input.fullRect.size + (input.safeRect.size - input.fullRect.size) * options.safeAreaFit
        };
        const float width = rc.width;
        const float height = rc.height;
        const float scale = fminf(width / options.baseResolution.x, height / options.baseResolution.y);
        output.offset = options.alignment * (rc.size - scale * options.baseResolution);
        output.scale = scale;
    } else {
        output.scale = length(options.pixelRatio);
        const rect_f rc{
                input.fullRect.position + (input.safeRect.position - input.fullRect.position) * options.safeAreaFit,
                input.fullRect.size + (input.safeRect.size - input.fullRect.size) * options.safeAreaFit
        };
        output.offset = options.alignment * (rc.size - output.scale * options.baseResolution);
    }

    output.offset += input.safeRect.position * options.safeAreaFit;

    const auto invScale = 1.0f / output.scale;
    output.fullRect = translate(input.fullRect, -output.offset) * invScale;
    output.safeRect = translate(input.safeRect, -output.offset) * invScale;
}

void updateViewport(ecs::entity e) {
    auto& vp = e.get<Viewport>();
    doScale(vp.input, vp.options, vp.output);

    auto& layout = e.get<LayoutRect>();
    layout.rect = vp.output.fullRect;
    layout.safeRect = vp.output.safeRect;
}

void Viewport::updateAll() {
    for (auto e : ecs::view<Viewport>()) {
        updateViewport(e);
    }
}

}
