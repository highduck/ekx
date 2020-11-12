#include "sample_base.hpp"
#include <ek/scenex/utility/scene_management.hpp>

namespace ek {

SampleBase::SampleBase() {
    container = create_node_2d("sample content");
    append(SampleBase::samplesContainer, container);

}

SampleBase::~SampleBase() {
    destroy_node(container);
}

void SampleBase::draw() {}

void SampleBase::update(float dt) {}

ecs::entity SampleBase::samplesContainer{};

}