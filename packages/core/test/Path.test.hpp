#include <doctest.h>

#include <ek/util/Path.hpp>

TEST_SUITE_BEGIN("c++ path");

TEST_CASE("join /") {
    String a = "a";
    String b = "b";
    String c = "c.txt";

    REQUIRE_EQ(strcmp((a / b).c_str(), "a/b"), 0);
    String abc = a / b / c;
    REQUIRE_EQ(strcmp(abc.c_str(), "a/b/c.txt"), 0);

}

TEST_SUITE_END();