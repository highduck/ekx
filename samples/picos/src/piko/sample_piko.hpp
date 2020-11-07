#pragma once

#include <sample_base.hpp>

namespace ek {

class SamplePiko : public SampleBase {
public:
    SamplePiko();
    ~SamplePiko() override;

    void draw() override;
    void update(float dt) override;

    ecs::entity container{};
};

}

