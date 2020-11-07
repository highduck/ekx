#pragma once

#include <ek/scenex/text/text_drawer.hpp>
#include "sample_base.hpp"

namespace ek {

struct SampleBlurProperties {
    float radius = 1.0f;
    int iterations = 1;
    int strength = 1;
    float4 color = float4::one;

    void gui();
};

class SampleText : public SampleBase {
public:
    SampleText();

    void draw() override;
    void update(float dt) override;

    static void prepareInternalResources();

public:
    TextDrawer textDrawer;
    SampleBlurProperties outline1;
    SampleBlurProperties outline2;
    SampleBlurProperties shadow;
    bool drawGlyphCache = false;
};

}

