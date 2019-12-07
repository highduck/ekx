#include <ek/util/base64.hpp>
#include <gtest/gtest.h>
#include <vector>

using namespace ek::base64;

void check(const std::vector<uint8_t>& data, const std::string& str) {
    auto encoded_str = encode(data);
    EXPECT_EQ(str, encoded_str);
    auto decoded_bytes = decode(encoded_str);
    EXPECT_EQ(data, decoded_bytes);
}

TEST(base64, encode_decode) {
    check({}, "");
    check({'f'}, "Zg==");
    check({'f', 'o'}, "Zm8=");
    check({'f', 'o', 'o'}, "Zm9v");
    check({'f', 'o', 'o', 'b'}, "Zm9vYg==");
    check({'f', 'o', 'o', 'b', 'a'}, "Zm9vYmE=");
    check({'f', 'o', 'o', 'b', 'a', 'r'}, "Zm9vYmFy");
    check({'f', 'o', 'o', '\0', 'b', 'a', 'r'}, "Zm9vAGJhcg==");
    check({'\0', '\0', '\0'}, "AAAA");
    check({'\0', '\0'}, "AAA=");
    check({'\0'}, "AA==");
}