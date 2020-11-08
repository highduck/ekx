#pragma once

#include <ek/scenex/text/text_drawer.hpp>
#include "sample_base.hpp"

namespace ek {

class SampleText : public SampleBase {
public:
    SampleText();

    void draw() override;
    void update(float dt) override;

    static void prepareInternalResources();

public:
    TextDrawer textDrawer;
    bool drawGlyphCache = false;
};

}

