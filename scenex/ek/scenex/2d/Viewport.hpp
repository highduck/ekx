#pragma once

#include <ek/math/box.hpp>

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
};

struct ViewportScaleInput {
    // screen metrics
    rect_f fullRect;
    rect_f safeRect;
    float dpiScale;
};

struct ViewportScaleOutput {
    rect_f fullRect;
    rect_f safeRect;
    float2 offset;
    float scale;
};

struct Viewport {
    ViewportScaleOptions options{};
    ViewportScaleOutput output{};
    ViewportScaleInput input{};

    Viewport() = default;

    Viewport(float width, float height) {
        options.baseResolution.x = width;
        options.baseResolution.y = height;
    }

    static void updateAll();
};


}