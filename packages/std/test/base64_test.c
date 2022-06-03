#include <unit.h>
#include <ek/base64.h>

static void base64_test(const char* data, unsigned N, const char* str) {
    char encoded_str[1024 + 1];
    uint32_t encoded_str_len = base64_encode(encoded_str, 1024, data, N);
    encoded_str[encoded_str_len] = 0;
    CHECK_EQ(strlen(str), encoded_str_len);
    CHECK_FALSE(strncmp(str, encoded_str, encoded_str_len));
    uint8_t decoded_bytes[1024 + 1];
    uint32_t decoded_bytes_len = base64_decode(decoded_bytes, 1024, encoded_str, encoded_str_len);
    decoded_bytes[decoded_bytes_len] = 0;
    CHECK_FALSE(strncmp(data, (const char*) decoded_bytes, N));
}

SUITE(base64) {
    IT("encode/decode") {
        base64_test("", 0, "");
        base64_test("f", 1, "Zg==");
        base64_test("fo", 2, "Zm8=");
        base64_test("foo", 3, "Zm9v");
        base64_test("foob", 4, "Zm9vYg==");
        base64_test("fooba", 5, "Zm9vYmE=");
        base64_test("foobar", 6, "Zm9vYmFy");
        base64_test("foo\0bar", 7, "Zm9vAGJhcg==");
        base64_test("\0\0\0", 3, "AAAA");
        base64_test("\0\0", 2, "AAA=");
        base64_test("\0", 1, "AA==");
    }
}
