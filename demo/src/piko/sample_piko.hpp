#pragma once

#include <sample_base.hpp>

namespace ek {

class SamplePiko : public SampleBase {
public:
    SamplePiko();

    void draw() override;

    void update(float dt) override;

    ecs::entity node{};
};

}

