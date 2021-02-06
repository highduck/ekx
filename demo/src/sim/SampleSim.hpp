#pragma once

#include <sample_base.hpp>

namespace ek {

class SampleSim : public SampleBase {
public:
    SampleSim();

    void draw() override;

    void update(float dt) override;
};

}
