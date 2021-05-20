#include <ek/util/Base64.hpp>
#include <gtest/gtest.h>
#include <cstring>



void check(const char* data, int N, const char* str) {
    using namespace ek::base64;
    char encoded_str[1024];
    auto encoded_str_len = encode(encoded_str, reinterpret_cast<const uint8_t*>(data), N);
    EXPECT_TRUE(strncmp(str, encoded_str, encoded_str_len) == 0);
    uint8_t decoded_bytes[1024];
    auto decoded_bytes_len = decode(decoded_bytes, encoded_str, encoded_str_len);
    EXPECT_TRUE(strncmp(data, reinterpret_cast<const char*>(decoded_bytes), N) == 0);
}

TEST(base64, encode_decode) {
    check({}, 0, "");
    check((char[1]){'f'}, 1, "Zg==");
    check((char[2]){'f', 'o'}, 2, "Zm8=");
    check((char[3]){'f', 'o', 'o'}, 3, "Zm9v");
    check((char[4]){'f', 'o', 'o', 'b'}, 4, "Zm9vYg==");
    check((char[5]){'f', 'o', 'o', 'b', 'a'}, 5, "Zm9vYmE=");
    check((char[6]){'f', 'o', 'o', 'b', 'a', 'r'}, 6, "Zm9vYmFy");
    check((char[7]){'f', 'o', 'o', '\0', 'b', 'a', 'r'}, 7, "Zm9vAGJhcg==");
    check((char[3]){'\0', '\0', '\0'}, 3, "AAAA");
    check((char[2]){'\0', '\0'}, 2, "AAA=");
    check((char[1]){'\0'}, 1, "AA==");
}