#include <ecxx/impl/identity_generator.hpp>
#include <gtest/gtest.h>

#include "identity_generator_shared.hpp"

using namespace ecxx;

TEST(identity_generator, test_2) {
    if (!i_generated) {
        i1 = identity_generator<ig_a, uint32_t>::value;
        i2 = identity_generator<ig_b, uint32_t>::value;
        i3 = identity_generator<ig_c, uint32_t>::value;
        i_generated = true;
    }

    ASSERT_EQ((identity_generator<ig_a, uint32_t>::value), i1);
    ASSERT_EQ((identity_generator<ig_b, uint32_t>::value), i2);
    ASSERT_EQ((identity_generator<ig_c, uint32_t>::value), i3);

    ASSERT_EQ((identity_generator<ig_a, uint32_t>::value), i1);
    ASSERT_EQ((identity_generator<ig_b, uint32_t>::value), i2);
    ASSERT_EQ((identity_generator<ig_c, uint32_t>::value), i3);

//    ASSERT_EQ((identity_generator<ig_c, uint16_t>::value), 0);
//    ASSERT_EQ((identity_generator<ig_a, uint16_t>::value), 1);
//    ASSERT_EQ((identity_generator<ig_b, uint16_t>::value), 2);
}
