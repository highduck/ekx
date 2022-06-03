#include <unit.h>
#include <ek/util/StringUtil.hpp>

SUITE(cxx_string) {
    IT("format_mm_ss") {
        using ek::format_time_mm_ss;
        REQUIRE_EQ(format_time_mm_ss(60).c_str(), "01:00");
        REQUIRE_EQ(format_time_mm_ss(60.05f, TIME_FORMAT_KEEP_HOURS).c_str(), "00:01:01");
    }
}
