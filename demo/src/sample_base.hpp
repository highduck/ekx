#pragma once

#include <string>
#include <ecxx/ecxx.hpp>

namespace ek {

class SampleBase {
public:
    SampleBase();

    virtual ~SampleBase();

    virtual void draw();

    virtual void update(float dt);

    std::string title;
    static ecs::entity samplesContainer;
    ecs::entity container;
private:
};

}

