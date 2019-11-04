#include <platform/boot.h>
#include <gtest/gtest.h>

using namespace ek;

TEST(dummy_test, pass) {

}

namespace ek {

void main() {
    auto args = get_program_c_arguments();
    int argc = args.size();
    char** argv = const_cast<char**>(args.data());
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::FLAGS_gtest_death_test_style = "fast";
    int exit_code = RUN_ALL_TESTS();
}

}

