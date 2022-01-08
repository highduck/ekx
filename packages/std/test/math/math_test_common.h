#ifndef MATH_TEST_COMMON_H
#define MATH_TEST_COMMON_H

#include <doctest.h>
#include <ek/math.h>
#include <stdio.h>

#define CHECK_NEAR_EQ_EPS(x, y, eps) do { \
    bool ok = almost_eq_f32((x),(y),(eps)); \
    if(!ok) printf("%s != %s : %f != %f", #x, #y, (x), (y)); \
    CHECK(ok); \
    } while(0)

#define CHECK_NEAR_EQ(x, y) CHECK_NEAR_EQ_EPS((x),(y),MATH_F32_EPSILON)

#endif // MATH_TEST_COMMON_H
