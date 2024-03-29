#include <doctest.h>
#include <ecxx/impl/world.hpp>
#include "identity_generator_shared.hpp"

using namespace ecs;

TEST_CASE("identity_generator, test_2") {

    using ecs::ComponentTypeIdGenerator;

    if (!idsGenerated) {
        idComponentA = ComponentTypeIdGenerator<ComponentA>::value;
        idComponentB = ComponentTypeIdGenerator<ComponentB>::value;
        idComponentC = ComponentTypeIdGenerator<ComponentC>::value;
        idsGenerated = true;
    }

    REQUIRE_EQ((ComponentTypeIdGenerator<ComponentC>::value), idComponentC);
    REQUIRE_EQ((ComponentTypeIdGenerator<ComponentB>::value), idComponentB);
    REQUIRE_EQ((ComponentTypeIdGenerator<ComponentA>::value), idComponentA);

    REQUIRE_EQ((ComponentTypeIdGenerator<ComponentA>::value), idComponentA);
    REQUIRE_EQ((ComponentTypeIdGenerator<ComponentB>::value), idComponentB);
    REQUIRE_EQ((ComponentTypeIdGenerator<ComponentC>::value), idComponentC);
}
