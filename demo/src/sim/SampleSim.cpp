#include "SampleSim.hpp"

#include <sim/driver_node_script.h>

namespace ek {

SampleSim::SampleSim() {
    sim::create(container);
    title = "SIMULATION";
}

void SampleSim::draw() {
    SampleBase::draw();
}

void SampleSim::update(float dt) {
    SampleBase::update(dt);
    sim::update();
}

}
