#pragma once

#include <ek/scenex/text/TextDrawer.hpp>
#include "sample_base.hpp"

namespace ek {

class SampleText : public SampleBase {
public:
    SampleText();

    void draw() override;

    void update(float dt) override;

    static void prepareInternalResources();

public:
};

}

