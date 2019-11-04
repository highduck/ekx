#include <ek/utility/base64.hpp>
#include <gtest/gtest.h>
#include <vector>

using namespace ek;

TEST(base64, encode) {
    EXPECT_EQ(base64_encode(""), "");
    EXPECT_EQ(base64_encode("f"), "Zg==");
    EXPECT_EQ(base64_encode("fo"), "Zm8=");
    EXPECT_EQ(base64_encode("foo"), "Zm9v");
    EXPECT_EQ(base64_encode("foob"), "Zm9vYg==");
    EXPECT_EQ(base64_encode("fooba"), "Zm9vYmE=");
    EXPECT_EQ(base64_encode("foobar"), "Zm9vYmFy");
}

TEST(base64, decode) {
    EXPECT_EQ(base64_decode_str(""), "");
    EXPECT_EQ(base64_decode_str("Zg=="), "f");
    EXPECT_EQ(base64_decode_str("Zm8="), "fo");
    EXPECT_EQ(base64_decode_str("Zm9v"), "foo");
    EXPECT_EQ(base64_decode_str("Zm9vYg=="), "foob");
    EXPECT_EQ(base64_decode_str("Zm9vYmE="), "fooba");
    EXPECT_EQ(base64_decode_str("Zm9vYmFy"), "foobar");
}