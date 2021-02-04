#include <ecxx/impl/world.hpp>
#include <gtest/gtest.h>
#include "identity_generator_shared.hpp"

using namespace ecs;

TEST(identity_generator, test_2) {

    using ecs::ComponentTypeIdGenerator;

    if (!idsGenerated) {
        idComponentA = ComponentTypeIdGenerator<ComponentA>::value;
        idComponentB = ComponentTypeIdGenerator<ComponentB>::value;
        idComponentC = ComponentTypeIdGenerator<ComponentC>::value;
        idsGenerated = true;
    }

    ASSERT_EQ((ComponentTypeIdGenerator<ComponentC>::value), idComponentC);
    ASSERT_EQ((ComponentTypeIdGenerator<ComponentB>::value), idComponentB);
    ASSERT_EQ((ComponentTypeIdGenerator<ComponentA>::value), idComponentA);

    ASSERT_EQ((ComponentTypeIdGenerator<ComponentA>::value), idComponentA);
    ASSERT_EQ((ComponentTypeIdGenerator<ComponentB>::value), idComponentB);
    ASSERT_EQ((ComponentTypeIdGenerator<ComponentC>::value), idComponentC);
}
