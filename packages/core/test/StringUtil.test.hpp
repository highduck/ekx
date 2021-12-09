#include <doctest.h>

#include <ek/util/StringUtil.hpp>

TEST_SUITE_BEGIN("c++ string");

TEST_CASE("format_mm_ss") {
    using ek::format_time_mm_ss;

    REQUIRE_EQ(format_time_mm_ss(60), "01:00");
    REQUIRE_EQ(format_time_mm_ss(60.05f, TimeFormat_KeepHours), "00:01:01");
}

TEST_SUITE_END();