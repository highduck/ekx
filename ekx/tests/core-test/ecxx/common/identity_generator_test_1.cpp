#include <ecxx/impl/world.hpp>
#include <gtest/gtest.h>
#include "identity_generator_shared.hpp"

TEST(identity_generator, test_1) {

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
