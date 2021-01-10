#include <sim/driver_node_script.h>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/base/Node.hpp>
#include "sample_piko.hpp"

namespace ek {

SamplePiko::SamplePiko() {
    auto& app = resolve<basic_application>();
    node = piko::create();
    append(container, node);
    title = "PIKO PIKO";
}

void SamplePiko::draw() {
    SampleBase::draw();
}

void SamplePiko::update(float dt) {
    SampleBase::update(dt);
    piko::update();
}

}