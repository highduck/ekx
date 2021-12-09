#ifndef STRING_TEST_H
#define STRING_TEST_H

#include <doctest.h>
#include <ek/string.h>

TEST_SUITE_BEGIN("string");

TEST_CASE("replace char") {
    char notFound[] = "Not Found";
    ek_cstr_replace(notFound, '%', '-');
    CHECK_FALSE(strcmp(notFound, "Not Found"));

    char hello1[] = "!Hello!";
    ek_cstr_replace(hello1, '!', '?');
    CHECK_FALSE(strcmp(hello1, "?Hello?"));

    char lll[] = "{X} == {X} == {X}";
    ek_cstr_replace(lll, 'X', 'z');
    CHECK_FALSE(strcmp(lll, "{z} == {z} == {z}"));
}

TEST_CASE("format_mm_ss") {
    char buf[32];
    ek_cstr_format_timer(buf, 32, 60000, 0);
    CHECK_FALSE(strcmp(buf, "01:00"));
    ek_cstr_format_timer(buf, 32, 60001, 1);
    CHECK_FALSE(strcmp(buf, "00:01:01"));
}

TEST_SUITE_END();

#endif // STRING_TEST_H
