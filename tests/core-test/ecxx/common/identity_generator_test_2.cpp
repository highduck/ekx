#include <ecxx/impl/utility.hpp>
#include <gtest/gtest.h>

#include "identity_generator_shared.hpp"

using namespace ecs::details;

TEST(identity_generator, test_2) {
    if (!i_generated) {
        i1 = identity_generator<ig_a>::value;
        i2 = identity_generator<ig_b>::value;
        i3 = identity_generator<ig_c>::value;
        i_generated = true;
    }

    ASSERT_EQ((identity_generator<ig_a>::value), i1);
    ASSERT_EQ((identity_generator<ig_b>::value), i2);
    ASSERT_EQ((identity_generator<ig_c>::value), i3);

    ASSERT_EQ((identity_generator<ig_a>::value), i1);
    ASSERT_EQ((identity_generator<ig_b>::value), i2);
    ASSERT_EQ((identity_generator<ig_c>::value), i3);

//    ASSERT_EQ((identity_generator<ig_c>::value), 0);
//    ASSERT_EQ((identity_generator<ig_a>::value), 1);
//    ASSERT_EQ((identity_generator<ig_b>::value), 2);
}
