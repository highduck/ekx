#pragma once

#include <ek/math/Rect.hpp>
#include <ek/util/Type.hpp>
#include <ecxx/ecxx_fwd.hpp>

namespace ek {

/*** Scale Camera viewport to Screen ***/
struct ViewportScaleOptions {
    // logical resolution size,
    Vec2f baseResolution = Vec2f::one;
    Vec2f alignment{0.5f, 0.5f};
    Vec2f safeAreaFit = Vec2f::one;
    // modes
    Vec2f pixelRatio = Vec2f::one;
    bool scaleToResolution = true;
    Rect2f viewport = Rect2f::zero_one;
};

struct ViewportScaleInput {
    // screen metrics
    Rect2f fullRect;
    Rect2f safeRect;
    float dpiScale;
};

struct ViewportScaleOutput {
    Rect2f screenRect;
    Rect2f fullRect;
    Rect2f safeRect;
    Vec2f offset;
    float scale;
};

struct GameDisplayInfo;

struct Viewport {
    ViewportScaleOptions options{};
    ViewportScaleOutput output{};

    Viewport() = default;

    Viewport(float width, float height) {
        options.baseResolution.x = width;
        options.baseResolution.y = height;
    }

    static void updateAll(const GameDisplayInfo& display0);
};

ECX_TYPE(17, Viewport);

}