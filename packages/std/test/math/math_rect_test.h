#ifndef MATH_RECT_TEST_H
#define MATH_RECT_TEST_H

#include "math_test_common.h"

TEST_SUITE_BEGIN("math: rects");

TEST_CASE("is empty") {
    CHECK(rect_is_empty(rect(0, 0, 0, 0)));
    CHECK(rect_is_empty(rect(1, 1, -1, -1)));
    CHECK(rect_is_empty(rect(0, 0, 0, 1)));
    CHECK(rect_is_empty(rect(0, 0, 1, 0)));
    CHECK_FALSE(rect_is_empty(rect(0, 0, 1, 1)));

    CHECK(brect_is_empty({{0, 0, 0, 0}}));
    CHECK(brect_is_empty(brect_inf()));
    CHECK(brect_is_empty({{1, 1, 1, 1}}));
    CHECK(brect_is_empty({{1, -1, 1, 1}}));
    CHECK(brect_is_empty({{-1, 1, 1, 1}}));
    CHECK(brect_is_empty({{-1, -1, -1, -1}}));
    CHECK_FALSE(brect_is_empty({{-1, -1, 1, 1}}));
}

TEST_SUITE_END();

#endif // MATH_RECT_TEST_H
