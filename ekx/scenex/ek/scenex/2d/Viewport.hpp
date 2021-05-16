#pragma once

#include <ek/math/box.hpp>
#include <ek/util/Type.hpp>

namespace ek {

/*** Scale Camera viewport to Screen ***/
struct ViewportScaleOptions {
    // logical resolution size,
    float2 baseResolution = float2::one;
    float2 alignment{0.5f, 0.5f};
    float2 safeAreaFit = float2::one;
    // modes
    float2 pixelRatio = float2::one;
    bool scaleToResolution = true;
    rect_f viewport = rect_f::zero_one;
};

struct ViewportScaleInput {
    // screen metrics
    rect_f fullRect;
    rect_f safeRect;
    float dpiScale;
};

struct ViewportScaleOutput {
    rect_f screenRect;
    rect_f fullRect;
    rect_f safeRect;
    float2 offset;
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

EK_DECLARE_TYPE(Viewport);

}