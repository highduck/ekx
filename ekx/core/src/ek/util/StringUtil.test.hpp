#include <doctest.h>

#include "StringUtil.hpp"

using std::string;

TEST_CASE("string_utility, replace") {
    using ek::replace;

    REQUIRE_EQ(replace("Not Found", "%NAME%", "World"), "Not Found");

    REQUIRE_EQ(replace("Hello, %NAME%!", "%NAME%", "World"), "Hello, World!");

    REQUIRE_EQ(replace("{X} == {X} == {X}", "{X}", "0"), "0 == 0 == 0");
}

TEST_CASE("string_utility, join") {
    using ek::join;

    REQUIRE_EQ(join({"A", "B", "C"}, ", "), "A, B, C");
    REQUIRE_EQ(join({}, ", "), "");
}

TEST_CASE("string_utility, split") {
    using ek::split;

    auto empty = split("", ' ');
    REQUIRE_EQ(1, empty.size());
    REQUIRE(empty.back().empty());

    auto nop = split("nop", ' ');
    REQUIRE_EQ(1, nop.size());
    REQUIRE_EQ(nop.back(), "nop");

    ek::Array<string> expected{"1", "2", "3"};
    ek::Array<string> actual = split("1;2;3", ';');
    REQUIRE_EQ(actual.size(), expected.size());
    for (size_t i = 0; i < actual.size(); ++i) {
        REQUIRE_EQ(actual[i], expected[i]);
    }
}

TEST_CASE("string_utility, format_mm_ss") {
    using ek::format_time_mm_ss;

    REQUIRE_EQ(format_time_mm_ss(60), "01:00");
    REQUIRE_EQ(format_time_mm_ss(60.05f), "01:01");
}

TEST_CASE("string_utility, hex_str") {
    using ek::to_hex;

    REQUIRE_EQ(to_hex(static_cast<uint8_t>(0x1)), "01");
    REQUIRE_EQ(to_hex(static_cast<uint16_t>(0xAB)), "00AB");
}

TEST_CASE("string_utility, starts_with") {
    using ek::starts_with;

    // test from abseil-cpp

    const std::string s1("123\0abc", 7);
    const std::string a("foobar");
    const std::string b(s1);
    const std::string e;
    REQUIRE(starts_with(a, a));
    REQUIRE(starts_with(a, "foo"));
    REQUIRE(starts_with(a, e));
    REQUIRE(starts_with(b, s1));
    REQUIRE(starts_with(b, b));
    REQUIRE(starts_with(b, e));
    REQUIRE(starts_with(e, ""));
    REQUIRE_FALSE(starts_with(a, b));
    REQUIRE_FALSE(starts_with(b, a));
    REQUIRE_FALSE(starts_with(e, a));

    // char spec
    REQUIRE(starts_with(s1, '1'));
    REQUIRE_FALSE(starts_with(s1, '2'));
    REQUIRE_FALSE(starts_with(e, '1'));
}

TEST_CASE("string_utility, ends_with") {
    using ek::ends_with;

    // test from abseil-cpp

    const std::string s1("123\0abc", 7);
    const std::string a("foobar");
    const std::string b(s1);
    const std::string e;
    REQUIRE(ends_with(a, a));
    REQUIRE(ends_with(a, "bar"));
    REQUIRE(ends_with(a, e));
    REQUIRE(ends_with(b, s1));
    REQUIRE(ends_with(b, b));
    REQUIRE(ends_with(b, e));
    REQUIRE(ends_with(e, ""));
    REQUIRE_FALSE(ends_with(a, b));
    REQUIRE_FALSE(ends_with(b, a));
    REQUIRE_FALSE(ends_with(e, a));

    // char spec
    REQUIRE(ends_with(s1, 'c'));
    REQUIRE_FALSE(ends_with(s1, '3'));
    REQUIRE_FALSE(ends_with(e, 'c'));
}
