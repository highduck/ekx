#include "Base64.hpp"
#include <doctest.h>
#include <cstring>

void check(const char* data, int N, const char* str) {
    using namespace ek::base64;
    char encoded_str[1024];
    auto encoded_str_len = encode(encoded_str, reinterpret_cast<const uint8_t*>(data), N);
    REQUIRE_EQ(strncmp(str, encoded_str, encoded_str_len), 0);
    uint8_t decoded_bytes[1024];
    auto decoded_bytes_len = decode(decoded_bytes, encoded_str, encoded_str_len);
    REQUIRE_EQ(strncmp(data, reinterpret_cast<const char*>(decoded_bytes), N), 0);
}

TEST_CASE("base64 encode_decode") {
    check("", 0, "");
    check("f", 1, "Zg==");
    check("fo", 2, "Zm8=");
    check("foo", 3, "Zm9v");
    check("foob", 4, "Zm9vYg==");
    check("fooba", 5, "Zm9vYmE=");
    check("foobar", 6, "Zm9vYmFy");
    check("foo\0bar", 7, "Zm9vAGJhcg==");
    check("\0\0\0", 3, "AAAA");
    check("\0\0", 2, "AAA=");
    check("\0", 1, "AA==");
}