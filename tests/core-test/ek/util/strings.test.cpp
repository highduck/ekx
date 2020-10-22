#include <ek/util/strings.hpp>
#include <gtest/gtest.h>

using std::vector;
using std::string;

TEST(string_utility, replace) {
    using ek::replace;

    ASSERT_STREQ(replace("Not Found", "%NAME%", "World").c_str(),
                 "Not Found");

    ASSERT_STREQ(replace("Hello, %NAME%!", "%NAME%", "World").c_str(),
                 "Hello, World!");

    ASSERT_STREQ("0 == 0 == 0",
                 replace("{X} == {X} == {X}", "{X}", "0").c_str());
}

TEST(string_utility, join) {
    using ek::join;

    ASSERT_STREQ(join({"A", "B", "C"}, ", ").c_str(), "A, B, C");
    ASSERT_STREQ(join({}, ", ").c_str(), "");
}

TEST(string_utility, split) {
    using ek::split;

    auto empty = split("", ' ');
    ASSERT_EQ(1, empty.size());
    ASSERT_TRUE(empty.back().empty());

    auto nop = split("nop", ' ');
    ASSERT_EQ(1, nop.size());
    ASSERT_STREQ(nop.back().c_str(), "nop");

    vector<string> expected{"1", "2", "3"};
    vector<string> actual = split("1;2;3", ';');
    ASSERT_EQ(actual.size(), expected.size());
    for (size_t i = 0; i < actual.size(); ++i) {
        EXPECT_STREQ(actual[i].c_str(), expected[i].c_str());
    }
}

TEST(string_utility, format_mm_ss) {
    using ek::format_time_mm_ss;

    ASSERT_STREQ(format_time_mm_ss(60).c_str(), "01:00");
    ASSERT_STREQ(format_time_mm_ss(60.05f).c_str(), "01:01");
}

TEST(string_utility, hex_str) {
    using ek::to_hex;

    ASSERT_STREQ(to_hex(static_cast<uint8_t>(0x1)).c_str(), "01");
    ASSERT_STREQ(to_hex(static_cast<uint16_t>(0xAB)).c_str(), "00AB");
}

TEST(string_utility, starts_with) {
    using ek::starts_with;

    // test from abseil-cpp

    const std::string s1("123\0abc", 7);
    const std::string a("foobar");
    const std::string b(s1);
    const std::string e;
    EXPECT_TRUE(starts_with(a, a));
    EXPECT_TRUE(starts_with(a, "foo"));
    EXPECT_TRUE(starts_with(a, e));
    EXPECT_TRUE(starts_with(b, s1));
    EXPECT_TRUE(starts_with(b, b));
    EXPECT_TRUE(starts_with(b, e));
    EXPECT_TRUE(starts_with(e, ""));
    EXPECT_FALSE(starts_with(a, b));
    EXPECT_FALSE(starts_with(b, a));
    EXPECT_FALSE(starts_with(e, a));

    // char spec
    EXPECT_TRUE(starts_with(s1, '1'));
    EXPECT_FALSE(starts_with(s1, '2'));
    EXPECT_FALSE(starts_with(e, '1'));
}

TEST(string_utility, ends_with) {
    using ek::ends_with;

    // test from abseil-cpp

    const std::string s1("123\0abc", 7);
    const std::string a("foobar");
    const std::string b(s1);
    const std::string e;
    EXPECT_TRUE(ends_with(a, a));
    EXPECT_TRUE(ends_with(a, "bar"));
    EXPECT_TRUE(ends_with(a, e));
    EXPECT_TRUE(ends_with(b, s1));
    EXPECT_TRUE(ends_with(b, b));
    EXPECT_TRUE(ends_with(b, e));
    EXPECT_TRUE(ends_with(e, ""));
    EXPECT_FALSE(ends_with(a, b));
    EXPECT_FALSE(ends_with(b, a));
    EXPECT_FALSE(ends_with(e, a));

    // char spec
    EXPECT_TRUE(ends_with(s1, 'c'));
    EXPECT_FALSE(ends_with(s1, '3'));
    EXPECT_FALSE(ends_with(e, 'c'));
}
