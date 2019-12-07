#include <platform/boot.hpp>
#include <gtest/gtest.h>

using namespace ek;

TEST(dummy_test, pass) {

}

namespace ek {

void main() {
    auto args = get_program_arguments();
    ::testing::InitGoogleTest(&args.argc, args.argv);
    ::testing::FLAGS_gtest_death_test_style = "fast";
    int exit_code = RUN_ALL_TESTS();
}

}

