#include <doctest.h>
#include <ek/base64.h>

TEST_SUITE_BEGIN("base64");

void check(const char* data, unsigned N, const char* str) {
    char encoded_str[1024 + 1];
    auto encoded_str_len = base64_encode(encoded_str, 1024, data, N);
    encoded_str[encoded_str_len] = 0;
    CHECK_EQ(strlen(str), encoded_str_len);
    CHECK_FALSE(strncmp(str, encoded_str, encoded_str_len));
    uint8_t decoded_bytes[1024 + 1];
    auto decoded_bytes_len = base64_decode(decoded_bytes, 1024, encoded_str, encoded_str_len);
    decoded_bytes[decoded_bytes_len] = 0;
    CHECK_FALSE(strncmp(data, (const char*) decoded_bytes, N));
}

TEST_CASE("encode/decode") {
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

TEST_SUITE_END();
