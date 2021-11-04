#include "Path.hpp"
#include <doctest.h>
#include <cstring>

TEST_CASE("path c") {
    char buf[3];

    ek::path_extract_dir(buf, 3, "/");
    REQUIRE_EQ(strcmp(buf, "/"), 0);

    ek::path_extract_dir(buf, 3, "a/b/c/");
    REQUIRE_EQ(strcmp(buf, "a/"), 0);

    ek::path_extract_dir(buf, 3, "dub.txt");
    REQUIRE_EQ(strcmp(buf, ""), 0);

    ek::path_extract_dir(buf, 3, "a/b/c/d");
    REQUIRE_EQ(strcmp(buf, "a/"), 0);

    ek::path_extract_dir(buf, 3, "a/b/c/dub.txt");
    REQUIRE_EQ(strcmp(buf, "a/"), 0);
}