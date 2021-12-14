#ifndef PATH_TEST_H
#define PATH_TEST_H

#include <doctest.h>
#include <ek/string.h>

TEST_SUITE_BEGIN("path");

TEST_CASE("dirname") {
    char buf[3];

    ek_path_dirname(buf, 3, "/");
    CHECK_FALSE(strcmp(buf, ""));

    ek_path_dirname(buf, 3, "a/b/c/");
    CHECK_FALSE(strcmp(buf, "a/"));

    ek_path_dirname(buf, 3, "dub.txt");
    CHECK_FALSE(strcmp(buf, ""));

    ek_path_dirname(buf, 3, "a/b/c/d");
    CHECK_FALSE(strcmp(buf, "a/"));

    ek_path_dirname(buf, 3, "a/b/c/dub.txt");
    CHECK_FALSE(strcmp(buf, "a/"));
}

TEST_CASE("join") {
    char buf[10];

    ek_path_join(buf, 10, "a/", "/b");
    CHECK_FALSE(strcmp(buf, "a/b"));

    char buf2[10];
    ek_path_join(buf2, 10, buf, "c.txt");
    CHECK_FALSE(strcmp(buf2, "a/b/c.txt"));

    ek_path_join(buf2, 10, "", "document.xml");
    CHECK_FALSE(strcmp(buf2, "document."));
}

TEST_CASE("ext") {
    CHECK_FALSE(strcmp(ek_path_ext("/D\\dir/a.txt"), "txt"));
    CHECK_FALSE(strcmp(ek_path_ext("a/b/c/d"), ""));
    CHECK_FALSE(ek_path_ext(0));
}

TEST_CASE("name") {

    CHECK_FALSE(strcmp(ek_path_name("/"), ""));
    CHECK_FALSE(strcmp(ek_path_name("/D\\dir/a.b.c.d.txt"), "a.b.c.d.txt"));
    CHECK_FALSE(strcmp(ek_path_ext("a/b/c/"), ""));
    CHECK_FALSE(ek_path_ext(0));
}

TEST_SUITE_END();

#endif // PATH_TEST_H
