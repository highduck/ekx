#include <gtest/gtest.h>
#include <ek/app/app.hpp>

using namespace ek::app;

TEST(dummy_test, pass) {

}

namespace ek {

void main() {
    auto args = g_app.args;
    ::testing::InitGoogleTest(&args.argc, args.argv);
    ::testing::FLAGS_gtest_death_test_style = "fast";
    int exit_code = RUN_ALL_TESTS();
}

}

