#include <unit.h>
#include <ek/string.h>

SUITE(path) {

    IT("has dirname") {
        char buf[3];

        ek_path_dirname(buf, 3, "/");
        CHECK_EQ(buf, "");

        ek_path_dirname(buf, 3, "a/b/c/");
        CHECK_EQ(buf, "a/");

        ek_path_dirname(buf, 3, "dub.txt");
        CHECK_EQ(buf, "");

        ek_path_dirname(buf, 3, "a/b/c/d");
        CHECK_EQ(buf, "a/");

        ek_path_dirname(buf, 3, "a/b/c/dub.txt");
        CHECK_EQ(buf, "a/");
    }

    IT("has join") {
        char buf[10];

        ek_path_join(buf, 10, "a/", "/b");
        CHECK_EQ(buf, "a/b");

        char buf2[10];
        ek_path_join(buf2, 10, buf, "c.txt");
        CHECK_EQ(buf2, "a/b/c.txt");

        ek_path_join(buf2, 10, "", "document.xml");
        CHECK_EQ(buf2, "document.");
    }

    IT("has ext") {
        CHECK_EQ(ek_path_ext("/D\\dir/a.txt"), "txt");
        CHECK_EQ(ek_path_ext("a/b/c/d"), "");
        CHECK_FALSE(ek_path_ext(0));
    }

    IT("has name") {
        CHECK_EQ(ek_path_name("/"), "");
        CHECK_EQ(ek_path_name("/D\\dir/a.b.c.d.txt"), "a.b.c.d.txt");
        CHECK_EQ(ek_path_ext("a/b/c/"), "");
        CHECK_FALSE(ek_path_ext(0));
    }
}
