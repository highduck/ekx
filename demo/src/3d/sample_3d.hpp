#pragma once

#include <sample_base.hpp>

namespace ek {

struct test_rotation_comp {
};

class Sample3D : public SampleBase {
public:
    Sample3D();
    ~Sample3D() override;
    void update(float dt) override;
    void draw() override;
};

}

