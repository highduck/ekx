#include <unit.h>
#include <ek/string.h>

SUITE(string) {

    IT("replace char")
    {
        char notFound[] = "Not Found";
        ek_cstr_replace(notFound, '%', '-');
        CHECK_EQ(notFound, "Not Found");

        char hello1[] = "!Hello!";
        ek_cstr_replace(hello1, '!', '?');
        CHECK_EQ(hello1, "?Hello?");

        char lll[] = "{X} == {X} == {X}";
        ek_cstr_replace(lll, 'X', 'z');
        CHECK_EQ(lll, "{z} == {z} == {z}");
    }

    IT("format_mm_ss")
    {
        char buf[32];
        ek_cstr_format_timer(buf, 32, 60000, 0);
        CHECK_EQ(buf, "01:00");
        ek_cstr_format_timer(buf, 32, 60001, 1);
        CHECK_EQ(buf, "00:01:01");
    }
}
