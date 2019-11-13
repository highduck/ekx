#include <scenex/2d/text_format.hpp>
#include <gtest/gtest.h>

TEST(dummy_test, pass) {
    scenex::text_format_t tf{"mini", 12};
    ASSERT_FLOAT_EQ(12.0f, tf.size);
}